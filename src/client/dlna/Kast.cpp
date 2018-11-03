#include "Kast.h"
#include "SSDPDiscovery.h"
#include "DLNAPlaybackInfo.h"
#include <QDebug>

Kast::Kast(QObject *parent) : QObject(parent)
{
    qDebug() << "Starting server...";

    m_fileServer = new HttpFileServer();
    m_fileServer->startServer();
    SSDPdiscovery *discovery = new SSDPdiscovery(this);
    connect(discovery, SIGNAL(foundRenderer(DLNARenderer*)), this, SLOT(onFoundRenderer(DLNARenderer*)));
    // Start SSDP discovery
    discovery->run();
}

void Kast::addItemToQueue(const QString &itemUrl)
{
    m_queue.append(itemUrl);
}

void Kast::play(const QString & renderer)
{
    auto it = m_renderers.find(renderer);
    if (m_renderers.end() != it)
    {
        (*it)->playPlayback();
    }
}

void Kast::pause(const QString & renderer)
{
    auto it = m_renderers.find(renderer);
    if (m_renderers.end() != it)
    {
        (*it)->pausePlayback();
    }
}

void Kast::stop(const QString & renderer)
{
    auto it = m_renderers.find(renderer);
    if (m_renderers.end() != it)
    {
        (*it)->stopPlayback();
    }
}

void Kast::onFoundRenderer(DLNARenderer *renderer)
{
    qDebug() << "Renderer found: " + renderer->getName();
    connect(renderer, SIGNAL(receivedResponse(QString,QString)), this, SLOT(onHttpResponse(QString,QString)));
    // Stop playback. Responses will be handled in handleResponse
    //renderer->stopPlayback();
    m_renderers[renderer->getName()] = renderer;
}

QHostAddress Kast::getLocalAddress()
{
    // see http://stackoverflow.com/questions/13835989/get-local-ip-address-in-qt
    for(auto && address : QNetworkInterface::allAddresses())
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol
                && address != QHostAddress(QHostAddress::LocalHost) // Check if it is local adress
                && address.toString().section( ".",-1,-1 ) != "1") // Check if it is virtual machine
             return address;
    }
    return QHostAddress();
}

QStringList Kast::getRenderers()
{
    return m_renderers.keys();
}

void Kast::setPlaybackUrl(const QString &renderer, const QUrl &url, const QFileInfo &fileInfo)
{
    auto it = m_renderers.find(renderer);
    if (m_renderers.end() != it)
    {
        (*it)->setPlaybackUrl(url, fileInfo);
    }
}

void Kast::setNextPlaybackUrl(const QString & renderer, const QUrl &url)
{
    auto it = m_renderers.find(renderer);
    if (m_renderers.end() != it)
    {
        (*it)->setNextPlaybackUrl(url);
    }
}

void Kast::onHttpResponse(const QString responseType, const QString data)
{
    Q_UNUSED(data); //May be needed in the future
    // Get renderer object
    DLNARenderer *renderer = qobject_cast<DLNARenderer *>(sender());
    qDebug() << "^Detected response type: "+responseType;

    // Handle responses
    if(responseType == "StopResponse")
    {

    }
    else if(responseType == "SetAVTransportURIResponse")
    {
        // Just play the playback url
        renderer->playPlayback();
    }
}


