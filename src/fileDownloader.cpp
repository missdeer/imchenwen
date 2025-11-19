/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

#include "fileDownloader.h"
#include "accessManager.h"

FileDownloader::FileDownloader(const QString &filepath, const QUrl &url, QObject *parent)
    : QObject(parent),
      m_sizeReply(nullptr),
      m_file(filepath),
      m_url(url),
      m_fileSize(-1),
      m_totalDownloaded(0),
      m_progress(0),
      m_threadCount(5),
      m_isMultiThreaded(false),
      m_state(Idle),
      m_fallbackGeneration(0)
{
    // Set progress file path
    m_progressFilePath = filepath + QStringLiteral(".progress");

    // Open file in ReadWrite mode for resume support
    if (m_file.exists())
    {
        if (!m_file.open(QFile::ReadWrite))
        {
            qDebug() << (QStringLiteral("Open file failed: ") + filepath);
            return;
        }
    }
    else
    {
        if (!m_file.open(QFile::WriteOnly))
        {
            qDebug() << (QStringLiteral("Create file failed: ") + filepath);
            return;
        }
    }

    // Try to load previous progress - this may populate m_chunks, m_fileSize, m_totalDownloaded
    // If no progress file exists, these remain empty/default and will be initialized in start()
    loadProgress();
}

FileDownloader::~FileDownloader()
{
    stop();
}

void FileDownloader::setState(State newState)
{
    if (m_state == newState)
    {
        return;
    }

    qDebug() << QStringLiteral("State transition: %1 -> %2").arg(m_state).arg(newState);
    m_state = newState;
}

// start a request
void FileDownloader::start()
{
    // Check current state
    switch (m_state)
    {
    case Downloading:
    case FetchingSize:
        qDebug() << "Already downloading";
        return;

    case InternalFallback:
        qDebug() << "Internal fallback in progress, ignoring external start";
        return;

    case Finished:
        qDebug() << "Download already finished";
        return;

    case Paused:
    case Idle:
    case Failed:
        // OK to start
        break;
    }

    // Reopen file if it was closed (e.g., after stop())
    if (!m_file.isOpen())
    {
        QIODevice::OpenMode mode = m_file.exists() ? QFile::ReadWrite : QFile::WriteOnly;
        if (!m_file.open(mode))
        {
            qDebug() << QStringLiteral("Failed to open file: %1").arg(m_file.errorString());
            setState(Failed);
            return;
        }
        qDebug()
            << QStringLiteral("Reopened file in %1 mode").arg(mode == QFile::ReadWrite ? QStringLiteral("ReadWrite") : QStringLiteral("WriteOnly"));
    }

    // Always reload progress from disk if a .progress file exists
    // The persisted state is the authoritative source of truth
    QFile progressFile(m_progressFilePath);
    if (progressFile.exists())
    {
        qDebug() << "Found progress file, reloading state from disk";
        loadProgress();
    }
    else if (m_chunks.isEmpty())
    {
        qDebug() << "No progress file found, starting fresh download";
    }
    else
    {
        qDebug() << "No progress file, using existing in-memory chunks (unusual)";
    }

    // Check if download is already complete (from loaded progress)
    if (!m_chunks.isEmpty() && allChunksFinished())
    {
        qDebug() << "Download already completed (from progress file)";
        // Validate file size if known
        if (m_fileSize > 0 && m_file.size() != m_fileSize)
        {
            qDebug() << QStringLiteral("Warning: File size mismatch - expected %1, got %2").arg(m_fileSize).arg(m_file.size());
        }
        // Clean up and signal completion
        QFile::remove(m_progressFilePath);
        m_file.close();
        setState(Finished);
        emit finished();
        return;
    }

    // Resume from existing chunks if available
    if (!m_chunks.isEmpty() && !allChunksFinished())
    {
        setState(Downloading);
        // Resume chunk downloads
        int activeCount = 0;
        for (int i = 0; i < m_chunks.size() && activeCount < m_threadCount; ++i)
        {
            if (!m_chunks[i].isFinished && m_chunks[i].reply == nullptr)
            {
                startChunkDownload(i);
                ++activeCount;
            }
        }
        emit started();
        return;
    }

    // Check if server supports range requests
    if (NetworkAccessManager::instance()->urlIsUnseekable(m_url))
    {
        // Server doesn't support range requests, use single-thread mode (1 chunk)
        m_isMultiThreaded = false;
        m_threadCount     = 1;
    }

    // Optimization: If single thread, skip size check and start downloading immediately
    // This saves one HTTP RTT. We will discover file size from the chunk response.
    if (m_threadCount == 1)
    {
        startSingleThreadDownload();
        return;
    }

    // Fetch file size first
    setState(FetchingSize);
    fetchFileSize();
    emit started();
}

// Pause
void FileDownloader::pause()
{
    if (m_state != Downloading && m_state != FetchingSize)
    {
        qDebug() << "Cannot pause, not downloading";
        return;
    }

    setState(Paused);

    if (m_sizeReply != nullptr)
    {
        m_sizeReply->abort();
    }

    // Pause all chunk downloads
    for (auto &chunk : m_chunks)
    {
        if (chunk.reply != nullptr)
        {
            chunk.reply->abort();
        }
    }

    // Save progress immediately on pause (state change)
    saveProgress();
}

// Stop
void FileDownloader::stop()
{
    bool isInternalFallback = (m_state == InternalFallback);

    // Increment generation to cancel any pending deferred restarts
    if (isInternalFallback)
    {
        ++m_fallbackGeneration;
        qDebug() << QStringLiteral("Canceling fallback restart, generation now %1").arg(m_fallbackGeneration);
    }

    // Stop size request
    if (m_sizeReply != nullptr)
    {
        m_sizeReply->disconnect();
        m_sizeReply->abort();
        m_sizeReply->deleteLater();
        m_sizeReply = nullptr;
    }

    // Stop all chunk downloads
    for (auto &chunk : m_chunks)
    {
        if (chunk.reply != nullptr)
        {
            chunk.reply->disconnect();
            chunk.reply->abort();
            chunk.reply->deleteLater();
            chunk.reply = nullptr;
        }
    }

    m_chunks.clear();

    // Only save progress and emit stopped() if NOT an internal fallback
    if (!isInternalFallback)
    {
        // External stop: save immediately regardless of throttle
        saveProgress();
        m_file.close();
        setState(Idle);
        emit stopped();
    }
    else
    {
        // Internal fallback: don't save stale progress (file was truncated)
        // or emit stopped() (to avoid client reentrancy)
        // Note: Progress is intentionally discarded during fallback recovery
        m_file.close();
        // Transition to Idle to allow user to restart
        setState(Idle);
    }
}

void FileDownloader::fetchFileSize()
{
    QNetworkRequest request(m_url);
    if (!m_cookie.isEmpty())
    {
        request.setRawHeader(QByteArrayLiteral("Cookie"), m_cookie);
    }
    request.setRawHeader(QByteArrayLiteral("Range"), QByteArrayLiteral("bytes=0-0"));

    m_sizeReply = NetworkAccessManager::instance()->head(request);
    if (m_sizeReply == nullptr)
    {
        // Fallback to GET request
        m_sizeReply = NetworkAccessManager::instance()->get(request);
    }

    connect(m_sizeReply, &QNetworkReply::finished, this, &FileDownloader::onSizeReplyFinished);
}

void FileDownloader::onSizeReplyFinished()
{
    Q_ASSERT(m_sizeReply);

    int status = m_sizeReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // Check for redirect
    if (status == 301 || status == 302)
    {
        m_sizeReply->deleteLater();
        m_url       = QString::fromUtf8(m_sizeReply->rawHeader(QByteArrayLiteral("Location")));
        m_sizeReply = nullptr;
        fetchFileSize();
        return;
    }

    // Get file size from Content-Range or Content-Length
    QString contentRange = QString::fromUtf8(m_sizeReply->rawHeader(QByteArrayLiteral("Content-Range")));
    if (!contentRange.isEmpty())
    {
        // Parse "bytes 0-0/1234567" format
        qsizetype slashPos = contentRange.indexOf(QLatin1Char('/'));
        if (slashPos != -1)
        {
            m_fileSize = contentRange.mid(slashPos + 1).toLongLong();
        }
    }
    else
    {
        m_fileSize = m_sizeReply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    }

    m_sizeReply->deleteLater();
    m_sizeReply = nullptr;

    // Check if server supports range requests (206 Partial Content)
    bool supportsRange = (status == 206) || (!contentRange.isEmpty());

    // Determine download strategy
    if (m_fileSize <= 0 || !supportsRange)
    {
        // Unknown size or no range support: use single open-ended chunk
        m_isMultiThreaded = false;
        m_threadCount     = 1;

        // Create single open-ended chunk
        m_chunks.clear();

        qint64 existingSize = m_file.size();
        qint64 startOffset  = 0;

        if (!supportsRange)
        {
            // CRITICAL: No range support means we cannot resume
            // Must truncate to avoid appending to stale partial data
            if (existingSize > 0)
            {
                qDebug() << QStringLiteral("No range support detected, truncating existing %1 bytes").arg(existingSize);
                if (!m_file.resize(0))
                {
                    qDebug() << QStringLiteral("Failed to truncate file: %1").arg(m_file.errorString());
                    return;
                }
                m_file.flush();
            }
            // Delete progress file - cannot resume without Range support
            QFile::remove(m_progressFilePath);
            m_totalDownloaded = 0;
            startOffset       = 0;
        }
        else
        {
            // Unknown size but range support exists: try to resume
            m_totalDownloaded = existingSize;
            startOffset       = existingSize;
        }

        ChunkInfo chunk {};
        chunk.startPos   = startOffset;
        chunk.endPos     = -1; // Open-ended (unknown)
        chunk.downloaded = 0;
        chunk.isFinished = false;
        chunk.reply      = nullptr;
        m_chunks.append(chunk);

        qDebug() << QStringLiteral("Using single-thread download (unknown size or no range support), starting at offset %1").arg(chunk.startPos);
        setState(Downloading);
        startChunkDownload(0);
    }
    else
    {
        // Known size and range support: use multi-threaded chunks
        m_isMultiThreaded = (m_threadCount > 1);

        createChunks();

        setState(Downloading);

        // Start downloading chunks
        int chunksToStart = qMin(m_threadCount, m_chunks.size());
        for (int i = 0; i < chunksToStart; ++i)
        {
            startChunkDownload(i);
        }

        qDebug() << QStringLiteral("Using %1-thread download for %2 bytes").arg(m_threadCount).arg(m_fileSize);
    }
}

void FileDownloader::onChunkMetadataChanged(int chunkIndex)
{
    if (chunkIndex < 0 || chunkIndex >= m_chunks.size())
    {
        return;
    }

    ChunkInfo &chunk = m_chunks[chunkIndex];
    if (chunk.reply == nullptr)
    {
        return;
    }

    int status = chunk.reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // CRITICAL: Check if server ignored our Range request for non-zero offset
    if (chunk.startPos > 0 && status == 200)
    {
        qDebug() << QStringLiteral("Server returned 200 instead of 206 for chunk %1 (offset %2) - server does not support Range")
                        .arg(chunkIndex)
                        .arg(chunk.startPos);

        // CRITICAL: Disconnect and delete the reply to prevent stray callbacks
        chunk.reply->disconnect();
        chunk.reply->abort();
        chunk.reply->deleteLater();
        chunk.reply = nullptr;

        qDebug() << "Truncating file to 0 and restarting without Range support";

        // Transition to InternalFallback state
        setState(InternalFallback);

        // Increment generation to invalidate any previous deferred restarts
        ++m_fallbackGeneration;
        int currentGeneration = m_fallbackGeneration;

        // CRITICAL: Truncate BEFORE closing the file
        if (m_file.isOpen())
        {
            m_file.resize(0);
            m_file.flush();
        }

        // Delete progress file - this prevents the restart from loading stale state
        QFile::remove(m_progressFilePath);

        // Mark URL as unseekable to prevent future Range requests
        NetworkAccessManager::instance()->addUnseekableHost(m_url.host());

        // Stop all downloads (closes file, sets state to Idle)
        stop();

        // Clear all state
        m_chunks.clear();
        m_totalDownloaded = 0;
        m_fileSize        = -1;

        // Reopen the file for writing
        if (!m_file.open(QFile::WriteOnly))
        {
            qDebug() << QStringLiteral("Failed to reopen file after truncation: %1").arg(m_file.fileName());
            setState(Failed);
            return;
        }

        // Set parameters for restart
        m_isMultiThreaded = false;
        m_threadCount     = 1;

        // CRITICAL: Defer restart to event loop to avoid reentrancy
        // Capture generation to detect if stop() was called
        QTimer::singleShot(0, this, [this, currentGeneration]() {
            // Check if this restart is still valid (not canceled by stop())
            if (m_fallbackGeneration != currentGeneration)
            {
                qDebug() << QStringLiteral("Fallback restart canceled (generation %1 != %2)").arg(currentGeneration).arg(m_fallbackGeneration);
                return;
            }

            // Check state is still Idle (stop() should have set it)
            if (m_state != Idle)
            {
                qDebug() << QStringLiteral("Fallback restart aborted, state is %1 not Idle").arg(m_state);
                return;
            }

            qDebug() << "Executing deferred fallback restart";
            start();
        });
    }
    // Also validate Content-Range header matches our request
    else if (status == 206 && chunk.endPos >= 0)
    {
        QString contentRange = QString::fromUtf8(chunk.reply->rawHeader(QByteArrayLiteral("Content-Range")));
        // Expected format: "bytes start-end/total"
        if (!contentRange.isEmpty())
        {
            qDebug() << QStringLiteral("Chunk %1 Content-Range: %2").arg(chunkIndex).arg(contentRange);
            // Could add strict validation here
        }
    }

    discoverFileSizeFromChunk(chunkIndex);
}

void FileDownloader::discoverFileSizeFromChunk(int chunkIndex)
{
    if (m_fileSize > 0)
    {
        return;
    }

    if (chunkIndex < 0 || chunkIndex >= m_chunks.size())
    {
        return;
    }

    const ChunkInfo &chunk = m_chunks[chunkIndex];
    if (chunk.reply == nullptr)
    {
        return;
    }

    int status = chunk.reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // Lazy size discovery: If we skipped fetchFileSize(), we might not know m_fileSize yet.
    // Try to learn it from Content-Range or Content-Length.
    QString contentRange = QString::fromUtf8(chunk.reply->rawHeader(QByteArrayLiteral("Content-Range")));
    if (!contentRange.isEmpty())
    {
        // Parse "bytes start-end/total"
        qsizetype slashPos = contentRange.indexOf(QLatin1Char('/'));
        if (slashPos != -1)
        {
            qint64 total = contentRange.mid(slashPos + 1).toLongLong();
            if (total > 0)
            {
                m_fileSize = total;
                qDebug() << QStringLiteral("Discovered file size from Content-Range: %1").arg(m_fileSize);

                // Update the open-ended chunk endPos if now known
                if (chunk.endPos == -1)
                {
                    // Note: We don't strictly enforce endPos for the single chunk case
                    // but it helps with progress calculation
                }
            }
        }
    }
    else if (status == 200)
    {
        // If 200 OK, Content-Length is the full file size
        qint64 len = chunk.reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
        if (len > 0)
        {
            m_fileSize = len;
            qDebug() << QStringLiteral("Discovered file size from Content-Length: %1").arg(m_fileSize);
        }
    }
}

void FileDownloader::startSingleThreadDownload()
{
    m_isMultiThreaded = false;

    // Create single chunk starting from current file size (resume if possible)
    // We optimistically assume Range support. If not supported, onChunkMetadataChanged
    // will handle the fallback (truncate and restart).
    m_chunks.clear();

    qint64 existingSize = m_file.size();

    ChunkInfo chunk {};
    chunk.startPos   = existingSize;
    chunk.endPos     = -1; // Unknown end
    chunk.downloaded = 0;
    chunk.isFinished = false;
    chunk.reply      = nullptr;
    m_chunks.append(chunk);

    qDebug() << QStringLiteral("Single thread optimization: skipping size check, starting at %1").arg(existingSize);

    setState(Downloading);
    startChunkDownload(0);
    emit started();
}

void FileDownloader::createChunks()
{
    // Only create chunks if we don't already have them (e.g., from loadProgress())
    if (!m_chunks.isEmpty())
    {
        qDebug() << "Chunks already exist (from progress file), skipping creation";
        return;
    }

    if (m_fileSize <= 0)
    {
        qDebug() << "File size unknown, cannot create chunks";
        return;
    }

    // Reset total downloaded when creating fresh chunks
    m_totalDownloaded = 0;

    // Calculate chunk size (minimum 1MB per chunk)
    constexpr qint64 minChunkSize = 1024LL * 1024;
    // Use 4x chunks per thread to improve concurrency (load balancing)
    int    targetChunkCount = m_threadCount * 4;
    qint64 chunkSize        = qMax(m_fileSize / targetChunkCount, minChunkSize);

    qint64 currentPos = 0;

    while (currentPos < m_fileSize)
    {
        ChunkInfo chunk {};
        chunk.startPos   = currentPos;                                       // IMMUTABLE boundary
        chunk.endPos     = qMin(currentPos + chunkSize - 1, m_fileSize - 1); // IMMUTABLE boundary
        chunk.downloaded = 0;
        chunk.isFinished = false;
        chunk.reply      = nullptr;

        m_chunks.append(chunk);
        currentPos = chunk.endPos + 1;
    }

    qDebug() << QStringLiteral("Created %1 chunks for %2 bytes").arg(m_chunks.size()).arg(m_fileSize);
}

void FileDownloader::startChunkDownload(int chunkIndex)
{
    if (chunkIndex < 0 || chunkIndex >= m_chunks.size())
    {
        return;
    }

    ChunkInfo &chunk = m_chunks[chunkIndex];

    if (chunk.isFinished || chunk.reply != nullptr)
    {
        return;
    }

    // Create range request based on current position
    QNetworkRequest request(m_url);
    if (!m_cookie.isEmpty())
    {
        request.setRawHeader(QByteArrayLiteral("Cookie"), m_cookie);
    }

    if (chunk.endPos >= 0)
    {
        // Bounded chunk: use "bytes=start-end"
        QByteArray rangeHeader =
            QByteArrayLiteral("bytes=") + QByteArray::number(chunk.currentPos()) + QByteArrayLiteral("-") + QByteArray::number(chunk.endPos);
        request.setRawHeader(QByteArrayLiteral("Range"), rangeHeader);
    }
    else if (chunk.currentPos() > 0)
    {
        // Open-ended chunk with resume: use "bytes=start-"
        QByteArray rangeHeader = QByteArrayLiteral("bytes=") + QByteArray::number(chunk.currentPos()) + QByteArrayLiteral("-");
        request.setRawHeader(QByteArrayLiteral("Range"), rangeHeader);
    }
    // else: no Range header for fresh open-ended download

    chunk.reply = NetworkAccessManager::instance()->get(request);
    if (chunk.reply == nullptr)
    {
        qDebug() << QStringLiteral("Failed to create network request for chunk %1").arg(chunkIndex);
        return;
    }

    // CRITICAL: For non-zero startPos chunks, we MUST validate the response before writing
    // Use metaDataChanged signal to check headers before readyRead
    connect(chunk.reply, &QNetworkReply::metaDataChanged, this, [this, chunkIndex]() { onChunkMetadataChanged(chunkIndex); });

    // Use lambda to capture chunk index
    connect(chunk.reply, &QNetworkReply::readyRead, this, [this, chunkIndex]() { onChunkReadyRead(chunkIndex); });

    connect(chunk.reply, &QNetworkReply::finished, this, [this, chunkIndex]() { onChunkFinished(chunkIndex); });
}

void FileDownloader::onChunkReadyRead(int chunkIndex)
{
    if (chunkIndex < 0 || chunkIndex >= m_chunks.size())
    {
        return;
    }

    ChunkInfo &chunk = m_chunks[chunkIndex];
    if (chunk.reply == nullptr)
    {
        return;
    }

    int status = chunk.reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (status != 200 && status != 206)
    {
        qDebug() << QStringLiteral("Aborting chunk %1 due to invalid status code: %2").arg(chunkIndex).arg(status);
        chunk.reply->abort();
        return;
    }

    QByteArray data = chunk.reply->readAll();
    if (data.isEmpty())
    {
        return;
    }

    // Write to file at correct position with thread safety
    {
        QMutexLocker locker(&m_fileMutex);

        // Seek to current position (startPos is IMMUTABLE, only downloaded changes)
        if (!m_file.seek(chunk.currentPos()))
        {
            qDebug() << QStringLiteral("Failed to seek to position %1 for chunk %2").arg(chunk.currentPos()).arg(chunkIndex);
            return;
        }

        qint64 written = m_file.write(data);
        if (written != data.size())
        {
            qDebug() << QStringLiteral("Partial write: expected %1 bytes, wrote %2 bytes for chunk %3").arg(data.size()).arg(written).arg(chunkIndex);
            if (written < 0)
            {
                return; // Write error
            }
        }

        // Update progress atomically inside mutex
        chunk.downloaded += written;
        m_totalDownloaded += written;
    }

    // Update progress and save outside of file mutex to avoid blocking other chunks
    updateProgress();

    // Throttle progress saves: only save every 2 seconds to reduce I/O
    if (!m_progressSaveTimer.isValid() || m_progressSaveTimer.elapsed() > 2000)
    {
        saveProgress();
        m_progressSaveTimer.restart();
    }
}

void FileDownloader::onChunkFinished(int chunkIndex)
{
    if (chunkIndex < 0 || chunkIndex >= m_chunks.size())
    {
        return;
    }

    ChunkInfo &chunk = m_chunks[chunkIndex];
    if (chunk.reply == nullptr)
    {
        return;
    }

    // Write remaining data
    int                         status      = chunk.reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QNetworkReply::NetworkError error       = chunk.reply->error();
    QString                     errorString = chunk.reply->errorString();

    // Only write remaining data if status is valid
    if (status == 200 || status == 206)
    {
        QByteArray remainingData = chunk.reply->readAll();
        if (!remainingData.isEmpty())
        {
            QMutexLocker locker(&m_fileMutex);
            if (m_file.seek(chunk.currentPos()))
            {
                qint64 written = m_file.write(remainingData);
                if (written > 0)
                {
                    chunk.downloaded += written;
                    m_totalDownloaded += written;
                }
                else if (written < 0)
                {
                    qDebug() << QStringLiteral("Write error in chunk %1: %2").arg(chunkIndex).arg(m_file.errorString());
                }
            }
        }
    }

    // Handle redirects
    if (status == 301 || status == 302)
    {
        QUrl redirectUrl = chunk.reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (redirectUrl.isValid())
        {
            m_url = redirectUrl;
            chunk.reply->deleteLater();
            chunk.reply = nullptr;
            // Retry this chunk with new URL
            startChunkDownload(chunkIndex);
            return;
        }
    }

    chunk.reply->deleteLater();
    chunk.reply = nullptr;

    // Check if chunk is finished
    if (error == QNetworkReply::NoError && (status == 200 || status == 206))
    {
        qint64 expectedSize = chunk.size();

        if (expectedSize < 0)
        {
            // Open-ended chunk: finished when server closes connection
            chunk.isFinished = true;
            qDebug() << QStringLiteral("Open-ended chunk %1 finished: %2 bytes downloaded").arg(chunkIndex).arg(chunk.downloaded);
        }
        else if (chunk.downloaded >= expectedSize)
        {
            chunk.isFinished = true;
        }
        else
        {
            qDebug() << QStringLiteral("Chunk %1 incomplete: downloaded %2 of %3 bytes").arg(chunkIndex).arg(chunk.downloaded).arg(expectedSize);
            // Could retry this chunk
        }
    }
    else if (error != QNetworkReply::OperationCanceledError)
    {
        qDebug() << QStringLiteral("Chunk %1 download error (status %2): %3").arg(chunkIndex).arg(status).arg(errorString);
        // Could implement retry logic here
    }

    // If paused, save and signal
    if (m_state == Paused)
    {
        // Save immediately on pause (state change)
        saveProgress();
        emit paused();
        return;
    }

    // Start next unfinished chunk to maintain thread pool
    for (int i = 0; i < m_chunks.size(); ++i)
    {
        if (!m_chunks[i].isFinished && m_chunks[i].reply == nullptr)
        {
            startChunkDownload(i);
            break;
        }
    }

    // Check if all chunks are finished
    if (allChunksFinished())
    {
        // Validate file size matches expected
        if (m_fileSize > 0 && m_file.size() != m_fileSize)
        {
            qDebug() << QStringLiteral("Warning: File size mismatch - expected %1, got %2").arg(m_fileSize).arg(m_file.size());
        }

        // Delete progress file - download complete
        QFile::remove(m_progressFilePath);
        m_file.close();
        setState(Finished);
        emit finished();
    }
    else
    {
        updateProgress();
        // Save immediately on chunk finish (state change)
        saveProgress();
    }
}

void FileDownloader::updateProgress()
{
    // Single unified progress calculation
    if (m_fileSize > 0)
    {
        // Percentage when total size is known
        m_progress = static_cast<int>((m_totalDownloaded * 100) / m_fileSize);
    }
    else
    {
        // MB downloaded when total size is unknown
        m_progress = static_cast<int>(m_totalDownloaded >> 20);
    }
    emit progressChanged(m_progress);
}

bool FileDownloader::allChunksFinished() const
{
    for (const auto &chunk : m_chunks)
    {
        if (!chunk.isFinished)
        {
            return false;
        }
    }
    return !m_chunks.isEmpty();
}

void FileDownloader::saveProgress()
{
    if (m_chunks.isEmpty())
    {
        return;
    }

    QJsonObject progressObj;
    progressObj[QStringLiteral("fileSize")]        = m_fileSize;
    progressObj[QStringLiteral("totalDownloaded")] = m_totalDownloaded;

    QJsonArray chunksArray;
    for (const auto &chunk : m_chunks)
    {
        QJsonObject chunkObj;
        chunkObj[QStringLiteral("startPos")]   = static_cast<qint64>(chunk.startPos);
        chunkObj[QStringLiteral("endPos")]     = static_cast<qint64>(chunk.endPos);
        chunkObj[QStringLiteral("downloaded")] = static_cast<qint64>(chunk.downloaded);
        chunkObj[QStringLiteral("isFinished")] = chunk.isFinished;
        chunksArray.append(chunkObj);
    }
    progressObj[QStringLiteral("chunks")] = chunksArray;

    QFile progressFile(m_progressFilePath);
    if (progressFile.open(QFile::WriteOnly))
    {
        QJsonDocument doc(progressObj);
        progressFile.write(doc.toJson());
        progressFile.close();
    }
}

void FileDownloader::loadProgress()
{
    QFile progressFile(m_progressFilePath);
    if (!progressFile.exists() || !progressFile.open(QFile::ReadOnly))
    {
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(progressFile.readAll());
    progressFile.close();

    if (doc.isNull() || !doc.isObject())
    {
        qDebug() << "Invalid progress file, ignoring";
        return;
    }

    QJsonObject progressObj = doc.object();
    m_fileSize              = progressObj[QStringLiteral("fileSize")].toVariant().toLongLong();
    m_totalDownloaded       = progressObj[QStringLiteral("totalDownloaded")].toVariant().toLongLong();
    QJsonArray chunksArray  = progressObj[QStringLiteral("chunks")].toArray();

    if (m_fileSize <= 0 || chunksArray.isEmpty())
    {
        qDebug() << "Progress file has invalid data, ignoring";
        return;
    }

    // Actually restore chunks from JSON
    m_chunks.clear();
    for (int i = 0; i < chunksArray.size(); ++i)
    {
        QJsonObject chunkObj = chunksArray[i].toObject();
        ChunkInfo   chunk {};
        chunk.startPos   = chunkObj[QStringLiteral("startPos")].toVariant().toLongLong();
        chunk.endPos     = chunkObj[QStringLiteral("endPos")].toVariant().toLongLong();
        chunk.downloaded = chunkObj[QStringLiteral("downloaded")].toVariant().toLongLong();
        chunk.isFinished = chunkObj[QStringLiteral("isFinished")].toBool();
        chunk.reply      = nullptr;

        m_chunks.append(chunk);
    }

    qDebug() << QStringLiteral("Loaded progress: %1/%2 bytes, %3 chunks").arg(m_totalDownloaded).arg(m_fileSize).arg(m_chunks.size());
}
