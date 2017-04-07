#include "streammanager.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

StreamManager::StreamManager(QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
    , m_nam(nam)
{

}

void StreamManager::startDownload(const QStringList &streams)
{
    for (int i=0; i < streams.length(); i++)
    {
        QNetworkRequest req;
        req.setUrl(QUrl(streams.at(i)));
        StreamReplyPtr r(new StreamReply(i, m_nam->get(req)));
        m_streams.push_back(r);
        m_localUrls.push_back(QString("http://127.0.0.1:9876/%1").arg(i));
    }
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
