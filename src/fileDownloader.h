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

#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <cstdint>

#include <QElapsedTimer>
#include <QFile>
#include <QList>
#include <QMutex>
#include <QPair>
#include <QUrl>

class QNetworkReply;

struct ChunkInfo
{
    qint64         startPos;   // IMMUTABLE - original chunk start boundary
    qint64         endPos;     // IMMUTABLE - original chunk end boundary
    qint64         downloaded; // MUTABLE - bytes downloaded for this chunk
    bool           isFinished;
    QNetworkReply *reply;

    // Helper methods
    [[nodiscard]] qint64 currentPos() const
    {
        return startPos + downloaded;
    }
    [[nodiscard]] qint64 remaining() const
    {
        return (endPos >= 0) ? (endPos - startPos + 1 - downloaded) : -1; // -1 = unknown
    }
    [[nodiscard]] qint64 size() const
    {
        return (endPos >= 0) ? (endPos - startPos + 1) : -1; // -1 = unknown (open-ended)
    }
};

class FileDownloader : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FileDownloader)

public:
    enum State : std::uint8_t
    {
        Idle,             // Not started or stopped
        FetchingSize,     // Fetching file size from server
        Downloading,      // Active download in progress
        Paused,           // Download paused by user
        InternalFallback, // Recovering from Range-ignored (internal restart)
        Finished,         // Download completed successfully
        Failed            // Download failed with error
    };
    Q_ENUM(State)

signals:
    void started();
    void paused();
    void stopped();
    void finished();
    void progressChanged(int progress);

public:
    FileDownloader(const QString &filepath, const QUrl &url, QObject *parent = nullptr);
    ~FileDownloader() override;
    void              pause();
    void              start();
    void              stop();
    [[nodiscard]] int progress() const
    {
        return m_progress;
    }
    void setThreadCount(int threadCount)
    {
        m_threadCount = threadCount;
    }
    void setCookie(const QByteArray &cookie)
    {
        m_cookie = cookie;
    }

private:
    void fetchFileSize();
    void createChunks();
    void startSingleThreadDownload();
    void discoverFileSizeFromChunk(int chunkIndex);
    void startChunkDownload(int chunkIndex);
    void onSizeReplyFinished();
    void onChunkMetadataChanged(int chunkIndex);
    void onChunkFinished(int chunkIndex);
    void onChunkReadyRead(int chunkIndex);
    void updateProgress();
    bool allChunksFinished() const;
    void saveProgress();
    void loadProgress();

    void setState(State newState);

    QNetworkReply   *m_sizeReply;
    QList<ChunkInfo> m_chunks;
    QFile            m_file;
    QMutex           m_fileMutex;
    QUrl             m_url;
    QByteArray       m_cookie;
    qint64           m_fileSize;
    qint64           m_totalDownloaded;
    int              m_progress;
    int              m_threadCount;
    bool             m_isMultiThreaded;
    State            m_state;
    int              m_fallbackGeneration; // Invalidates stale deferred restarts
    QString          m_progressFilePath;
    QElapsedTimer    m_progressSaveTimer; // Throttle progress saves
};

#endif // FILEDOWNLOADER_H
