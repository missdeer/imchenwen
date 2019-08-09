#include "inmemoryhandler.h"
#include "browser.h"
#include "util.h"
#include <qhttpengine/socket.h>
#include <qhttpengine/qiodevicecopier.h>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QFileInfo>
#include <QUuid>
#include <QStandardPaths>
#include <QTimer>

using namespace QHttpEngine;

InMemoryHandler::InMemoryHandler(QObject *parent)
    : Handler(parent)
    , m_inputEnd(false)
    , m_mediaSocket(nullptr)
{
    m_localAddress = Util::getLocalAddress().toString();
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

QString InMemoryHandler::mapUrl(const QString &url)
{
    QUrl u(url);
    QFileInfo fi(u.path());
    QString ext = fi.suffix();

    QString path = QString("%1.%2").arg(QUuid::createUuid().toString(QUuid::WithoutBraces), ext);
    m_1to1UrlMap.insert(path, url);

    return QString("http://%1:51290/%2").arg(m_localAddress, path);
}

void InMemoryHandler::clear()
{
    auto replys = m_replySocketMap.keys();
    for (auto r : replys)
        r->abort();
    m_m3u8.clear();
    m_mediaData.clear();
    m_referrer.clear();
    m_userAgent.clear();
    m_1to1UrlMap.clear();
}

void InMemoryHandler::returnMediaM3U8(Socket *socket)
{
    socket->setHeader("Content-Length", QByteArray::number(m_m3u8.length()));
    socket->setHeader("Content-Type", "application/vnd.apple.mpegurl");
    socket->writeHeaders();
    socket->write(m_m3u8);
}

void InMemoryHandler::returnMediaData(Socket *socket)
{
    m_inputEnd = false;
    m_mediaSocket = socket;
    socket->setHeader("Content-Type", "application/octet-stream");
    socket->writeHeaders();
    if (!m_mediaData.isEmpty())
    {
        qint64 writtenBytes = socket->write(m_mediaData);
        m_mediaData.remove(0, static_cast<int>(writtenBytes));
    }
    if (m_mediaData.isEmpty())
        QTimer::singleShot(20, this, &InMemoryHandler::relayMediaData);
    else
        QTimer::singleShot(10, this, &InMemoryHandler::relayMediaData);
}

void InMemoryHandler::relayMedia(Socket *socket, const QString &url)
{
    QNetworkRequest req;
    QUrl u(url);
    req.setUrl(u);
    if (!m_userAgent.isEmpty())
        req.setRawHeader("User-Agent", m_userAgent);
    if (!m_referrer.isEmpty())
        req.setRawHeader("Referer", m_referrer);
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    req.setAttribute(QNetworkRequest::HTTP2AllowedAttribute, true);

    qDebug() << __FUNCTION__ << url << QString(m_userAgent) << QString(m_referrer);
    QNetworkAccessManager &nam = Browser::instance().networkAccessManager();
    QNetworkReply *reply = nam.get(req);
    m_replySocketMap.insert(reply, socket);
    connect(reply, &QIODevice::readyRead, this, &InMemoryHandler::onReadyRead);
    connect(reply, &QNetworkReply::finished, this, &InMemoryHandler::onMediaReadFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &InMemoryHandler::onNetworkError);
    connect(reply, &QNetworkReply::sslErrors, this, &InMemoryHandler::onNetworkSSLErrors);
}

void InMemoryHandler::process(Socket *socket, const QString &path)
{
    qDebug() << __FUNCTION__ << path;
    if (path == "media.m3u8")
    {
        if (m_m3u8.isEmpty())
        {
            socket->writeError(Socket::NotFound);
            return;
        }
        returnMediaM3U8(socket);
        return;
    }

    if (path == "media.ts")
    {
        returnMediaData(socket);
        return;
    }

    auto it1to1 = m_1to1UrlMap.find(path);
    if (m_1to1UrlMap.end() != it1to1)
    {
        qDebug() << __FUNCTION__ << it1to1.key() << it1to1.value();
        relayMedia(socket, it1to1.value());
        return;
    }

    QString fullPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + path;
    if (QFile::exists(fullPath))
    {
        serveFileSystemFile(socket, fullPath);
        return;
    }

    socket->writeError(Socket::NotFound);
    qDebug() << __FUNCTION__ << path << "not found";
}

void InMemoryHandler::newMediaData(const QByteArray &data)
{
    m_inputEnd = false;
    m_mediaData.append(data);
}

void InMemoryHandler::onInputEnd()
{
    m_inputEnd = true;
}

void InMemoryHandler::onNetworkError(QNetworkReply::NetworkError code)
{
    auto reply = qobject_cast<QNetworkReply*>(sender());
    Socket* socket = m_replySocketMap[reply];
    socket->writeError(code);
}

void InMemoryHandler::onNetworkSSLErrors(const QList<QSslError> &errors)
{
    auto reply = qobject_cast<QNetworkReply*>(sender());
    for (auto & e : errors)
        qWarning() << e.errorString();
    Socket* socket = m_replySocketMap[reply];
    socket->writeError(Socket::InternalServerError);
}

void InMemoryHandler::onReadyRead()
{
    auto reply = qobject_cast<QNetworkReply*>(sender());
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
        qDebug() << __FUNCTION__ << statusCode;
        socket->writeError(statusCode);
    }
}

void InMemoryHandler::onMediaReadFinished()
{
    qDebug() << __FUNCTION__;
    onReadyRead();

    auto reply = qobject_cast<QNetworkReply*>(sender());
    Socket* socket = m_replySocketMap[reply];
    socket->close();
    m_replySocketMap.remove(reply);
    m_headerWritten.remove(reply);
    reply->deleteLater();
}

void InMemoryHandler::relayMediaData()
{
    if (!m_mediaData.isEmpty() && m_mediaSocket)
    {
        qint64 writtenBytes = m_mediaSocket->write(m_mediaData);
        m_mediaData.remove(0, static_cast<int>(writtenBytes));
    }
    if (!m_inputEnd)
    {
        if (m_mediaData.isEmpty())
            QTimer::singleShot(20, this, &InMemoryHandler::relayMediaData);
        else
            QTimer::singleShot(10, this, &InMemoryHandler::relayMediaData);
    }
    else if (m_mediaData.isEmpty())
    {
        if (m_mediaSocket)
        {
            m_mediaSocket->close();
            m_mediaSocket = nullptr;
        }
    }
}

void InMemoryHandler::serveFileSystemFile(Socket *socket, const QString &absolutePath)
{
    // Attempt to open the file for reading
    auto file = new QFile(absolutePath);
    if (!file->open(QIODevice::ReadOnly)) {
        delete file;
        qDebug() << __FUNCTION__ << "can't open file for reading:" << absolutePath;
        socket->writeError(Socket::Forbidden);
        return;
    }

    // Create a QIODeviceCopier to copy the file contents to the socket
    auto copier = new QIODeviceCopier(file, socket);
    connect(copier, &QIODeviceCopier::finished, copier, &QIODeviceCopier::deleteLater);
    connect(copier, &QIODeviceCopier::finished, file, &QFile::deleteLater);
    connect(copier, &QIODeviceCopier::finished, [socket]() {
        socket->close();
    });

    // Stop the copier if the socket is disconnected
    connect(socket, &Socket::disconnected, copier, &QIODeviceCopier::stop);

    // range is not supported
    // don't set Content-Length, it's a stream, length is unknown
    // special Content-Type for DMR
    socket->setHeader("Content-Type", "application/octet-stream");
    socket->writeHeaders();

    // Start the copy
    copier->start();

    qDebug() << __FUNCTION__ << absolutePath;
}
