#include "streammanager.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <qhttpserver.h>
#include <qhttprequest.h>
#include <qhttpresponse.h>

StreamManager::StreamManager(QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
    , m_nam(nam)
    , m_server(new QHttpServer)
    , m_downloadIndex(0)
{
    connect(m_server, SIGNAL(newRequest(QHttpRequest*, QHttpResponse*)),
            this, SLOT(handle(QHttpRequest*, QHttpResponse*)));
}

StreamManager::~StreamManager()
{
    shutdown();
    delete m_server;
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
    QStringList a = addr.split(':');
    QHostAddress host(a[0]);

    m_server->listen(host, a[1].toInt() );
}

void StreamManager::shutdown()
{
    m_server->close();
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

void StreamManager::handle(QHttpRequest *req, QHttpResponse *resp)
{
    QString path = req->path();
    while(path.at(0) == '/')path.remove(0, 1);
    int index = path.toInt();

    auto it = std::find_if(m_streams.begin(), m_streams.end(), [index](StreamReplyPtr s){ return s->index() == index;});
    if (m_streams.end() != it)
    {
        StreamReplyPtr r(*it);
        resp->writeHead(r->statusCode());
        auto pairs = r->rawHeaderPairs();
        for (auto p : pairs)
        {
            resp->setHeader(QString(p.first), QString(p.second));
        }
        while (!r->atEnd())
        {
            resp->write(r->read());
        }
    }

    resp->end();
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
