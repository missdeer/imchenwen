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

#include <QFileInfo>
#include <QSettings>

#include "playlistModel.h"
#include "dialogs.h"
#include "mpvObject.h"
#include "parserLux.h"
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
        m_audioTrackUrls << fileUrls[1]; // Second url is the audio stream
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
            m_subtitleUrls << QUrl();
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
    Q_ASSERT(ParserYtdlp::instance() != nullptr);

    // Select parser
    QSettings settings;
    Parser    parser = static_cast<Parser>(settings.value(QStringLiteral("player/parser")).toInt());
    if (parser == Parser::LUX || (parser == Parser::AUTO && isSupportedByLux(url.host())))
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
    beginRemoveRows(QModelIndex(), index, index);
    m_titles.removeAt(index);
    m_fileUrls.removeAt(index);
    m_danmakuUrls.removeAt(index);
    m_audioTrackUrls.removeAt(index);
    m_subtitleUrls.removeAt(index);
    endRemoveRows();
}

void PlaylistModel::clear()
{
    if (m_titles.isEmpty())
    {
        return;
    }
    beginRemoveRows(QModelIndex(), 0, m_titles.count() - 1);
    m_titles.clear();
    m_fileUrls.clear();
    m_danmakuUrls.clear();
    m_audioTrackUrls.clear();
    m_subtitleUrls.clear();
    endRemoveRows();
}

void PlaylistModel::playItem(int index)
{
    Q_ASSERT(MpvObject::instance() != nullptr);

    if (index >= 0 && index < m_titles.count())
    {
        MpvObject::instance()->open(m_fileUrls[index], m_danmakuUrls[index], m_audioTrackUrls[index], m_subtitleUrls[index]);
    }
    if (m_playingIndex != index)
    {
        m_playingIndex = index;
        emit playingIndexChanged();
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
