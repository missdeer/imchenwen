#include "streammanager.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QStandardPaths>
#include <QFile>

StreamManager::StreamManager(QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
    , m_nam(nam)
    , m_downloadIndex(0)
    , m_finishedCount(0)
    , m_runningCount(0)
{
}

StreamManager::~StreamManager()
{
}

void StreamManager::startDownload(const QStringList &streams)
{
    m_remoteUrls = streams;
    for (int c = 0; c < streams.length(); c++)
        m_localUrls.push_back( QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).arg(c));
    m_finishedCount = m_runningCount = m_downloadIndex = 0;
    download(m_downloadIndex++);
}

void StreamManager::stopDownload()
{
    for ( auto s : m_streams)
    {
        s->stop();
    }
    m_streams.clear();
    for ( auto u : m_localUrls)
    {
        QFile::remove(u);
    }
    m_localUrls.clear();
}

const QStringList &StreamManager::urls()
{
    return m_localUrls;
}

void StreamManager::finished()
{
    m_runningCount--;
    m_finishedCount++;
    int maxRunningCount = 1;
    for (int i = 4; i >=0; i--)
    {
        if ((m_finishedCount*2) >= (i *(i+1)))
        {
            maxRunningCount = i+1;
            break;
        }
    }
    qDebug() << "===========running:" << m_runningCount
             << ", finished:" << m_finishedCount
             << ", index:" << m_downloadIndex
             << ", max running:" << maxRunningCount;
    while (m_runningCount < maxRunningCount && m_downloadIndex < m_remoteUrls.length())
    {
        download(m_downloadIndex++);
    }
}

void StreamManager::download(int i)
{
    qDebug() << __FUNCTION__ << i;
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
    m_runningCount++;
}
