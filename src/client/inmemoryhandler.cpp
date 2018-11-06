#include "inmemoryhandler.h"
#include <qhttpengine/socket.h>

using namespace QHttpEngine;

InMemoryHandler::InMemoryHandler(QObject *parent)
    : Handler(parent)
{

}

void InMemoryHandler::setM3U8(const QByteArray &m3u8)
{
    m_m3u8 = m3u8;
}

void InMemoryHandler::process(Socket *socket, const QString &path)
{
    if (m_m3u8.isEmpty())
    {
        socket->writeError(Socket::NotFound);
        return;
    }

    if (path == "media.m3u8")
    {
        socket->setHeader("Content-Length", QByteArray::number(m_m3u8.length()));
        socket->setHeader("Content-Type", "application/vnd.apple.mpegurl");
        socket->writeHeaders();
        socket->write(m_m3u8);
    }
}
