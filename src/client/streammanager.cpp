#include "streammanager.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

StreamManager::StreamManager(QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
    , m_nam(nam)
    , m_downloadIndex(0)
{

}

void StreamManager::startDownload(const QStringList &streams)
{
    m_remoteUrls = streams;
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

void StreamManager::serve(const QString &addr)
{

}

void StreamManager::shutdown()
{

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
    req.setUrl(QUrl(m_remoteUrls.at(i)));
    StreamReply* sr = new StreamReply(i, m_nam->get(req));
    connect(sr, &StreamReply::done, this, &StreamManager::finished);
    StreamReplyPtr r(sr);
    m_streams.push_back(r);
    m_localUrls.push_back(QString("http://127.0.0.1:9876/%1").arg(i));
}
