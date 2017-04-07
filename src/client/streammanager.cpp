#include "streammanager.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QStandardPaths>

StreamManager::StreamManager(QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
    , m_nam(nam)
    , m_downloadIndex(0)
{
}

StreamManager::~StreamManager()
{
}

void StreamManager::startDownload(const QStringList &streams)
{
    m_remoteUrls = streams;
    for (int c = 0; c < streams.length(); c++)
    m_localUrls.push_back( QString("%1/imchenwencache-%2").arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).arg(c));
    m_downloadIndex = 0;
    download(m_downloadIndex++);
}

void StreamManager::stopDownload()
{
    for ( auto s : m_streams)
    {
        s->stop();
    }
    m_streams.clear();
    m_localUrls.clear();
}

const QStringList &StreamManager::urls()
{
    return m_localUrls;
}

void StreamManager::finished()
{
    int t = m_downloadIndex;
    for (int c = 0; c < t+1; c++)
    {
        if (m_downloadIndex < m_remoteUrls.length())
        {
            download(m_downloadIndex++);
        }
    }
}

void StreamManager::download(int i)
{
    QNetworkRequest req;
    req.setUrl(QUrl::fromUserInput(m_remoteUrls.at(i)));
    StreamReply* sr = new StreamReply(i, m_nam->get(req));
    connect(sr, &StreamReply::done, this, &StreamManager::finished);
    if (i == 0)
    {
        connect(sr, &StreamReply::localReadyRead, this, &StreamManager::readyRead);
    }
    StreamReplyPtr r(sr);
    m_streams.push_back(r);
}
