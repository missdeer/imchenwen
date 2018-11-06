#include "inmemoryhandler.h"
#include <qhttpengine/socket.h>
#include <qhttpengine/qiodevicecopier.h>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>

using namespace QHttpEngine;

InMemoryHandler::InMemoryHandler(QObject *parent)
    : Handler(parent)
{
    m_nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

void InMemoryHandler::setM3U8(const QByteArray &m3u8)
{
    m_m3u8 = m3u8;
}

void InMemoryHandler::setReferrer(const QByteArray &referrer)
{
    m_referrer = referrer;
}

void InMemoryHandler::setUserAgent(const QByteArray &userAgent)
{
    m_userAgent = userAgent;
}

void InMemoryHandler::returnMediaM3U8(Socket *socket)
{
    socket->setHeader("Content-Length", QByteArray::number(m_m3u8.length()));
    socket->setHeader("Content-Type", "application/vnd.apple.mpegurl");
    socket->writeHeaders();
    socket->write(m_m3u8);
}

void InMemoryHandler::relayMedia(Socket *socket, const QString &url)
{
    QNetworkRequest req;
    QUrl u(url);
    req.setUrl(u);
    req.setRawHeader("User-Agent", m_userAgent);
    req.setRawHeader("Referer", m_referrer);
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QNetworkReply* reply = m_nam.get(req);
    m_replySocketMap.insert(reply, socket);
    connect(reply, &QIODevice::readyRead, this, &InMemoryHandler::onReadyRead);
    connect(reply, &QNetworkReply::finished, this, &InMemoryHandler::onReadFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &InMemoryHandler::onNetworkError);
    connect(reply, &QNetworkReply::sslErrors, this, &InMemoryHandler::onNetworkSSLErrors);
}

void InMemoryHandler::process(Socket *socket, const QString &path)
{
    m_socket = socket;
    if (m_m3u8.isEmpty())
    {
        socket->writeError(Socket::NotFound);
        return;
    }

    if (path == "media.m3u8")
    {
        returnMediaM3U8(socket);
        return;
    }

    QString rawPath = socket->rawPath();
    if (rawPath.startsWith("/s/"))
    {
        QString url = "https://" + rawPath.mid(3);
        relayMedia(socket, url);
    }
    else
    {
        QString url = "http:/" + rawPath;
        relayMedia(socket, url);
    }
}

void InMemoryHandler::onNetworkError(QNetworkReply::NetworkError code)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    Socket* socket = m_replySocketMap[reply];
    socket->writeError(code);
}

void InMemoryHandler::onNetworkSSLErrors(const QList<QSslError> &errors)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    for (auto & e : errors)
        qWarning() << e.errorString();
    Socket* socket = m_replySocketMap[reply];
    socket->writeError(Socket::InternalServerError);
}

void InMemoryHandler::onReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    Socket* socket = m_replySocketMap[reply];

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 200 && statusCode < 300)
    {
        if (!m_headerWritten.contains(reply))
        {
            m_headerWritten.insert(reply);
            socket->setHeader("Content-Type", reply->rawHeader("Content-Type"));
            socket->setHeader("Content-Length", reply->rawHeader("Content-Length"));
            socket->writeHeaders();
        }
        socket->write(reply->readAll());
    }
    else if (statusCode >= 400)
    {
        socket->writeError(statusCode);
    }
}

void InMemoryHandler::onReadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();

    Socket* socket = m_replySocketMap[reply];
    socket->write(reply->readAll());
    socket->close();
    m_replySocketMap.remove(reply);
    m_headerWritten.remove(reply);
}
