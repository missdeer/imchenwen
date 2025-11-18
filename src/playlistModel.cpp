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

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

#include "playlistModel.h"
#include "dialogs.h"
#include "fileDownloader.h"
#include "mpvObject.h"
#include "parserLux.h"
#include "parserYKDL.h"
#include "parserYouGet.h"
#include "parserYoutubeDL.h"
#include "parserYtdlPatch.h"
#include "parserYtdlp.h"

namespace
{
    bool isSupportedByLux(const QString &domain)
    {
        static QSet<QString> domains {
            QStringLiteral("www.bilibili.com"),  QStringLiteral("www.douyin.com"),
            QStringLiteral("bcy.net"),           QStringLiteral("www.pixivision.net"),
            QStringLiteral("www.youku.com"),     QStringLiteral("m.toutiao.com"),
            QStringLiteral("v.ixigua.com"),      QStringLiteral("www.ixigua.com"),
            QStringLiteral("www.iqiyi.com"),     QStringLiteral("www.xinpianchang.com"),
            QStringLiteral("www.mgtv.com"),      QStringLiteral("www.tangdou.com"),
            QStringLiteral("v.douyu.com"),       QStringLiteral("www.miaopai.com"),
            QStringLiteral("weibo.com"),         QStringLiteral("v.qq.com"),
            QStringLiteral("music.163.com"),     QStringLiteral("yinyuetai.com"),
            QStringLiteral("time.geekbang.org"), QStringLiteral("haokan.baidu.com"),
            QStringLiteral("www.acfun.cn"),      QStringLiteral("hupu.com"),
            QStringLiteral("v.huya.com"),        QStringLiteral("www.kuaishou.com"),
            QStringLiteral("zhihu.com"),         QStringLiteral("xiaohongshu.com"),
        };

        auto iter = domains.find(domain);
        return iter != domains.end();
    }
} // namespace

PlaylistModel *PlaylistModel::s_instance = nullptr;

PlaylistModel::PlaylistModel(QObject *parent) : QAbstractListModel(parent), m_playingIndex(-1)
{
    Q_ASSERT(s_instance == nullptr);
    s_instance = this;
}

// Add item to playlist
void PlaylistModel::addItem(const QString &title, const QUrl &fileUrl, const QUrl &danmakuUrl, const QUrl &audioTrackUrl, const QUrl &subtitleUrl)
{
    int index = m_titles.count();
    beginInsertRows(QModelIndex(), index, index);
    m_titles << title;
    m_fileUrls << fileUrl;
    m_danmakuUrls << danmakuUrl;
    m_audioTrackUrls << audioTrackUrl;
    m_subtitleUrls << subtitleUrl;
    m_resolvedAudioPaths << QString();    // Not resolved yet
    m_resolvedSubtitlePaths << QString(); // Not resolved yet
    endInsertRows();
    playItem(index);
}

void PlaylistModel::addItems(const QString &title, const QList<QUrl> &fileUrls, const QUrl &danmakuUrl, const QUrl &subtitleUrl, bool isDash)
{
    int start = m_titles.count();

    if (isDash) // Youtube's dash videos
    {
        beginInsertRows(QModelIndex(), start, start);
        m_titles << title;
        m_fileUrls << fileUrls[0]; // First url is the video stream
        m_danmakuUrls << danmakuUrl;
        m_subtitleUrls << subtitleUrl;
        m_audioTrackUrls << fileUrls[1];   // Second url is the audio stream
        m_resolvedAudioPaths << QString(); // Not resolved yet
        m_resolvedSubtitlePaths << QString();
        endInsertRows();
    }
    else // Normal videos
    {
        int count = fileUrls.count();
        beginInsertRows(QModelIndex(), start, start + count - 1);
        for (int i = 0; i < count; i++)
        {
            m_titles << (title + QLatin1Char('_') + QString::number(i));
            m_fileUrls << fileUrls[i];
            m_danmakuUrls << (i == 0 ? danmakuUrl : QUrl());
            m_subtitleUrls << (i == 0 ? subtitleUrl : QUrl());
            m_audioTrackUrls << QUrl();
            m_resolvedAudioPaths << QString();
            m_resolvedSubtitlePaths << QString();
        }
        endInsertRows();
    }
    playItem(start);
}

void PlaylistModel::addLocalFiles(const QList<QUrl> &fileUrls)
{
    int start = m_titles.count();
    int count = fileUrls.count();

    beginInsertRows(QModelIndex(), start, start + count - 1);
    for (const auto &fileUrl : fileUrls)
    {
        m_titles << QFileInfo(fileUrl.toLocalFile()).fileName();
        m_fileUrls << fileUrl;
        m_audioTrackUrls << QUrl();

        QFile danmakuFile(fileUrl.toLocalFile() + QStringLiteral(".danmaku"));
        if (danmakuFile.open(QFile::ReadOnly | QFile::Text))
        {
            m_danmakuUrls << QString::fromUtf8(danmakuFile.readAll());
            danmakuFile.close();
        }
        else
        {
            m_danmakuUrls << QUrl();
        }

        m_subtitleUrls << QUrl();
        m_resolvedAudioPaths << QString();
        m_resolvedSubtitlePaths << QString();
    }
    endInsertRows();

    // Play video
    if (QSettings().value(QStringLiteral("player/autoplay")).toBool())
    {
        playItem(start);
    }
    else
    {
        MpvObject::instance()->showText(QByteArrayLiteral("File added"));
    }
}

void PlaylistModel::addUrl(const QUrl &url, bool download)
{
    Q_ASSERT(ParserLux::instance() != nullptr);
    Q_ASSERT(ParserYKDL::instance() != nullptr);
    Q_ASSERT(ParserYouGet::instance() != nullptr);
    Q_ASSERT(ParserYtdlp::instance() != nullptr);
    Q_ASSERT(ParserYtdlPatch::instance() != nullptr);
    Q_ASSERT(ParserYoutubeDL::instance() != nullptr);

    // Select parser
    if (isSupportedByLux(url.host()))
    {
        ParserLux::instance()->parse(url, download);
    }
    else
    {
        ParserYtdlp::instance()->parse(url, download);
    }
}

void PlaylistModel::addUrl(const QUrl &url)
{
    Q_ASSERT(Dialogs::instance() != nullptr);
    QSettings     settings;
    OpenUrlAction action = static_cast<OpenUrlAction>(settings.value(QStringLiteral("player/url_open_mode")).toInt());
    if (action == OpenUrlAction::QUESTION)
    {
        Dialogs::instance()->openUrlDialog(url);
    }
    else
    {
        addUrl(url, action == OpenUrlAction::DOWNLOAD);
    }
}

void PlaylistModel::removeItem(int index)
{
    if (index < 0 || index >= m_titles.length())
    {
        return;
    }

    // CRITICAL: If pending playback would be affected by this removal, cancel it entirely
    // This avoids the index-shift/lambda-capture mismatch bug where callbacks see
    // a shifted m_pendingPlayIndex and treat themselves as stale
    bool cancelPending = false;
    if (m_pendingPlayIndex >= 0)
    {
        if (m_pendingPlayIndex == index)
        {
            // Removing the pending item itself
            cancelPending = true;
        }
        else if (m_pendingPlayIndex > index)
        {
            // Removing an item below pending: pending index will shift
            // Lambdas captured old index, will see mismatch and bail
            cancelPending = true;
        }
    }

    if (cancelPending)
    {
        // Cancel ALL pending downloads (they have stale index captures)
        for (auto *downloader : m_audioDownloaders)
        {
            if (downloader)
            {
                downloader->disconnect();
                downloader->stop();
                downloader->deleteLater();
            }
        }
        m_audioDownloaders.clear();

        for (auto *downloader : m_subtitleDownloaders)
        {
            if (downloader)
            {
                downloader->disconnect();
                downloader->stop();
                downloader->deleteLater();
            }
        }
        m_subtitleDownloaders.clear();

        m_pendingPlayIndex = -1;
        m_playRequestToken++; // Invalidate stale callbacks

        qDebug() << "Canceled pending playback due to removal affecting indices";
    }
    else
    {
        // No pending playback or removal doesn't affect it
        // Only cancel downloads for THIS index
        if (m_audioDownloaders.contains(index))
        {
            auto *downloader = m_audioDownloaders.take(index);
            downloader->disconnect();
            downloader->stop();
            downloader->deleteLater();
        }
        if (m_subtitleDownloaders.contains(index))
        {
            auto *downloader = m_subtitleDownloaders.take(index);
            downloader->disconnect();
            downloader->stop();
            downloader->deleteLater();
        }

        // Remap downloader indices above the removed index (shift down)
        // This is safe because no pending playback is affected
        QHash<int, FileDownloader *> newAudioDownloaders;
        for (auto it = m_audioDownloaders.begin(); it != m_audioDownloaders.end(); ++it)
        {
            int oldIndex = it.key();
            if (oldIndex > index)
            {
                newAudioDownloaders[oldIndex - 1] = it.value(); // Shift down
            }
            else
            {
                newAudioDownloaders[oldIndex] = it.value(); // Keep same
            }
        }
        m_audioDownloaders = newAudioDownloaders;

        QHash<int, FileDownloader *> newSubtitleDownloaders;
        for (auto it = m_subtitleDownloaders.begin(); it != m_subtitleDownloaders.end(); ++it)
        {
            int oldIndex = it.key();
            if (oldIndex > index)
            {
                newSubtitleDownloaders[oldIndex - 1] = it.value();
            }
            else
            {
                newSubtitleDownloaders[oldIndex] = it.value();
            }
        }
        m_subtitleDownloaders = newSubtitleDownloaders;
    }

    // Remove from ALL parallel lists (indices shift automatically)
    beginRemoveRows(QModelIndex(), index, index);
    m_titles.removeAt(index);
    m_fileUrls.removeAt(index);
    m_danmakuUrls.removeAt(index);
    m_audioTrackUrls.removeAt(index);
    m_subtitleUrls.removeAt(index);
    m_resolvedAudioPaths.removeAt(index);
    m_resolvedSubtitlePaths.removeAt(index);
    endRemoveRows();
}

void PlaylistModel::clear()
{
    if (m_titles.isEmpty())
    {
        return;
    }

    // Cancel all downloads and pending playback
    cancelAllDownloads();
    m_pendingPlayIndex = -1;
    m_playRequestToken++;

    beginRemoveRows(QModelIndex(), 0, m_titles.count() - 1);
    m_titles.clear();
    m_fileUrls.clear();
    m_danmakuUrls.clear();
    m_audioTrackUrls.clear();
    m_subtitleUrls.clear();
    m_resolvedAudioPaths.clear();
    m_resolvedSubtitlePaths.clear();
    endRemoveRows();
}

void PlaylistModel::playItem(int index)
{
    Q_ASSERT(MpvObject::instance() != nullptr);

    if (index >= 0 && index < m_titles.count())
    {
        // Cancel any pending playback from previous call
        cancelPendingPlayback();

        m_pendingPlayIndex = index;
        m_playRequestToken++; // Invalidate old callbacks
        int currentToken = m_playRequestToken;

        // Resolve URLs (download if needed, use cache if available)
        resolveTrackUrls(index, currentToken);
    }
    else
    {
        // Invalid index, just update state
        if (m_playingIndex != index)
        {
            m_playingIndex = index;
            emit playingIndexChanged();
        }
    }
}

void PlaylistModel::playNextItem()
{
    playItem(m_playingIndex + 1);
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_titles.count();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    switch (role)
    {
    case TitleRole:
        return m_titles[row];
    }
    return {};
}

QHash<int, QByteArray> PlaylistModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TitleRole] = QByteArrayLiteral("title");
    return roles;
}

// Helper: Check if URL is from YouTube
bool PlaylistModel::isYouTubeUrl(const QUrl &url)
{
    if (!url.isValid() || url.isEmpty())
    {
        return false;
    }

    QString host = url.host();
    return host.endsWith(QStringLiteral("googlevideo.com")) || host.endsWith(QStringLiteral("youtube.com"));
}

// Helper: Generate stable temp file path using SHA-256 hash
QString PlaylistModel::generateTempPath(const QUrl &url, const QString &extension)
{
    // Use SHA-256 for stable, collision-resistant hash
    QByteArray urlBytes = url.toString().toUtf8();
    QByteArray hash     = QCryptographicHash::hash(urlBytes, QCryptographicHash::Sha256);

    // Use first 128 bits (32 hex chars) to avoid collisions
    // Birthday paradox: 50% collision at ~2^64 files with 128-bit hash
    QString hashHex = QString::fromLatin1(hash.toHex().left(32));

    // Extension must include dot
    Q_ASSERT(extension.startsWith(QLatin1Char('.')));

    return QDir::tempPath() + QStringLiteral("/imchenwen_track_") + hashHex + extension;
}

// Helper: Check if track URL is resolved to local file
bool PlaylistModel::isResolved(int index, TrackType type)
{
    if (index < 0 || index >= m_resolvedAudioPaths.size())
    {
        return false;
    }

    QString cachedPath = (type == TrackType::Audio) ? m_resolvedAudioPaths[index] : m_resolvedSubtitlePaths[index];

    if (cachedPath.isEmpty())
    {
        return false; // Not resolved yet
    }

    // Verify file still exists and is non-empty
    QFileInfo fileInfo(cachedPath);
    if (!fileInfo.exists() || fileInfo.size() == 0)
    {
        qDebug() << "Cached file missing or empty:" << cachedPath;

        // Clear stale cache entry
        if (type == TrackType::Audio)
        {
            m_resolvedAudioPaths[index] = QString();
        }
        else
        {
            m_resolvedSubtitlePaths[index] = QString();
        }
        return false;
    }

    return true; // Valid cached file
}

// Helper: Get resolved URL (local file if available, else original)
QUrl PlaylistModel::getResolvedUrl(int index, TrackType type)
{
    if (index < 0 || index >= m_resolvedAudioPaths.size())
    {
        return QUrl();
    }

    QString resolvedPath = (type == TrackType::Audio) ? m_resolvedAudioPaths[index] : m_resolvedSubtitlePaths[index];

    if (!resolvedPath.isEmpty() && QFile::exists(resolvedPath))
    {
        return QUrl::fromLocalFile(resolvedPath);
    }

    // Fallback to original URL
    return (type == TrackType::Audio) ? m_audioTrackUrls[index] : m_subtitleUrls[index];
}

// Cancel all active downloads
void PlaylistModel::cancelAllDownloads()
{
    // Cancel all audio downloads
    for (auto *downloader : m_audioDownloaders)
    {
        if (downloader)
        {
            downloader->disconnect();
            downloader->stop();
            downloader->deleteLater();
        }
    }
    m_audioDownloaders.clear();

    // Cancel all subtitle downloads
    for (auto *downloader : m_subtitleDownloaders)
    {
        if (downloader)
        {
            downloader->disconnect();
            downloader->stop();
            downloader->deleteLater();
        }
    }
    m_subtitleDownloaders.clear();
}

// Cancel pending playback request
void PlaylistModel::cancelPendingPlayback()
{
    if (m_pendingPlayIndex < 0)
    {
        return; // Nothing pending
    }

    int oldIndex = m_pendingPlayIndex;

    // Stop audio download if active
    if (m_audioDownloaders.contains(oldIndex))
    {
        auto *downloader = m_audioDownloaders.take(oldIndex);
        downloader->disconnect();
        downloader->stop();
        downloader->deleteLater();
    }

    // Stop subtitle download if active
    if (m_subtitleDownloaders.contains(oldIndex))
    {
        auto *downloader = m_subtitleDownloaders.take(oldIndex);
        downloader->disconnect();
        downloader->stop();
        downloader->deleteLater();
    }

    qDebug() << "Cancelled pending playback for index:" << oldIndex;
}

// Resolve audio URL (download if YouTube, cache if available)
void PlaylistModel::resolveAudioUrl(int index, int requestToken)
{
    if (index < 0 || index >= m_audioTrackUrls.size())
    {
        return;
    }

    QUrl    originalUrl = m_audioTrackUrls[index];
    QString tempPath    = generateTempPath(originalUrl, QStringLiteral(".m4a"));

    // Check if already cached and valid
    QFileInfo fileInfo(tempPath);
    if (fileInfo.exists() && fileInfo.size() > 0)
    {
        qDebug() << "Using cached audio track:" << tempPath;
        m_resolvedAudioPaths[index] = tempPath;
        checkResolutionCompleteAndPlay(index, requestToken);
        return;
    }

    // Start download
    qDebug() << "Downloading audio track to:" << tempPath;
    auto *downloader          = new FileDownloader(tempPath, originalUrl, this);
    m_audioDownloaders[index] = downloader;

    connect(downloader, &FileDownloader::finished, this, [this, index, requestToken, tempPath, downloader]() {
        // Validate request token first
        if (requestToken != m_playRequestToken)
        {
            qDebug() << "Audio download completed for stale request, ignoring";
            downloader->disconnect();
            downloader->deleteLater();
            m_audioDownloaders.remove(index);
            return;
        }

        // Check download success
        QFileInfo fileInfo(tempPath);
        if (fileInfo.exists() && fileInfo.size() > 0)
        {
            qDebug() << "Audio download completed:" << tempPath;

            // Store in parallel list (bounds check for safety)
            if (index >= 0 && index < m_resolvedAudioPaths.size())
            {
                m_resolvedAudioPaths[index] = tempPath;
            }
        }
        else
        {
            qDebug() << "Audio download failed or empty file:" << tempPath;
        }

        // Cleanup
        downloader->disconnect();
        downloader->deleteLater();
        m_audioDownloaders.remove(index);

        // Check if ready to play
        checkResolutionCompleteAndPlay(index, requestToken);
    });

    downloader->setThreadCount(3);
    downloader->start();
}

// Resolve subtitle URL (download if YouTube, cache if available)
void PlaylistModel::resolveSubtitleUrl(int index, int requestToken)
{
    if (index < 0 || index >= m_subtitleUrls.size())
    {
        return;
    }

    QUrl    originalUrl = m_subtitleUrls[index];
    QString tempPath    = generateTempPath(originalUrl, QStringLiteral(".vtt"));

    // Check cache
    QFileInfo fileInfo(tempPath);
    if (fileInfo.exists() && fileInfo.size() > 0)
    {
        qDebug() << "Using cached subtitle:" << tempPath;
        m_resolvedSubtitlePaths[index] = tempPath;
        checkResolutionCompleteAndPlay(index, requestToken);
        return;
    }

    // Start download
    qDebug() << "Downloading subtitle to:" << tempPath;
    auto *downloader             = new FileDownloader(tempPath, originalUrl, this);
    m_subtitleDownloaders[index] = downloader;

    connect(downloader, &FileDownloader::finished, this, [this, index, requestToken, tempPath, downloader]() {
        // Validate token
        if (requestToken != m_playRequestToken)
        {
            qDebug() << "Subtitle download completed for stale request, ignoring";
            downloader->disconnect();
            downloader->deleteLater();
            m_subtitleDownloaders.remove(index);
            return;
        }

        // Check success
        QFileInfo fileInfo(tempPath);
        if (fileInfo.exists() && fileInfo.size() > 0)
        {
            qDebug() << "Subtitle download completed:" << tempPath;

            if (index >= 0 && index < m_resolvedSubtitlePaths.size())
            {
                m_resolvedSubtitlePaths[index] = tempPath;
            }
        }
        else
        {
            qDebug() << "Subtitle download failed:" << tempPath;
        }

        // Cleanup
        downloader->disconnect();
        downloader->deleteLater();
        m_subtitleDownloaders.remove(index);

        // Check if ready to play
        checkResolutionCompleteAndPlay(index, requestToken);
    });

    downloader->setThreadCount(1);
    downloader->start();
}

// Check if all resolution is complete and play if ready
void PlaylistModel::checkResolutionCompleteAndPlay(int index, int requestToken)
{
    // Validate token
    if (requestToken != m_playRequestToken || index != m_pendingPlayIndex)
    {
        qDebug() << "Resolution check for stale request, ignoring";
        return;
    }

    // Check if both tracks are resolved (or don't need resolution)
    QUrl audioUrl    = m_audioTrackUrls[index];
    QUrl subtitleUrl = m_subtitleUrls[index];

    bool audioReady    = !isYouTubeUrl(audioUrl) || isResolved(index, TrackType::Audio);
    bool subtitleReady = !isYouTubeUrl(subtitleUrl) || isResolved(index, TrackType::Subtitle);

    // Also allow fallback: if download failed but we have original URL
    if (!audioReady && !m_audioDownloaders.contains(index))
    {
        qDebug() << "Audio download failed, will use original URL";
        audioReady = true;
    }
    if (!subtitleReady && !m_subtitleDownloaders.contains(index))
    {
        qDebug() << "Subtitle download failed, will use original URL";
        subtitleReady = true;
    }

    if (audioReady && subtitleReady)
    {
        playNow(index, requestToken);
    }
    else
    {
        qDebug() << "Waiting for track resolution... audio:" << audioReady << "subtitle:" << subtitleReady;
    }
}

// Play item now with resolved URLs
void PlaylistModel::playNow(int index, int requestToken)
{
    // Validate token
    if (requestToken != m_playRequestToken || index != m_pendingPlayIndex)
    {
        qDebug() << "Stale playback request, ignoring";
        return;
    }

    // Get resolved URLs (use local path if available, else original)
    QUrl audioUrl    = getResolvedUrl(index, TrackType::Audio);
    QUrl subtitleUrl = getResolvedUrl(index, TrackType::Subtitle);

    MpvObject::instance()->open(m_fileUrls[index], m_danmakuUrls[index], audioUrl, subtitleUrl);

    // Update playing index and clear pending state
    if (m_playingIndex != index)
    {
        m_playingIndex = index;
        emit playingIndexChanged();
    }
    m_pendingPlayIndex = -1;
}

// Resolve track URLs and start playback when ready
void PlaylistModel::resolveTrackUrls(int index, int requestToken)
{
    QUrl audioUrl    = m_audioTrackUrls[index];
    QUrl subtitleUrl = m_subtitleUrls[index];

    bool needAudioResolve    = isYouTubeUrl(audioUrl) && !isResolved(index, TrackType::Audio);
    bool needSubtitleResolve = isYouTubeUrl(subtitleUrl) && !isResolved(index, TrackType::Subtitle);

    if (!needAudioResolve && !needSubtitleResolve)
    {
        // Ready to play now
        playNow(index, requestToken);
        return;
    }

    // Start async resolution
    if (needAudioResolve)
    {
        resolveAudioUrl(index, requestToken);
    }
    if (needSubtitleResolve)
    {
        resolveSubtitleUrl(index, requestToken);
    }
}
