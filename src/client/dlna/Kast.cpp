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

void Kast::addItemToQueue(QString &itemUrl)
{
    m_queue.append(itemUrl);
}

void Kast::onFoundRenderer(DLNARenderer *renderer)
{
    qDebug() << "Renderer found: " + renderer->getName();
    connect(renderer, SIGNAL(receivedResponse(QString,QString)), this, SLOT(onHttpResponse(QString,QString)));
    // Stop playback. Responses will be handled in handleResponse
    renderer->stopPlayback();
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

void Kast::onHttpResponse(const QString responseType, const QString data)
{
    Q_UNUSED(data); //May be needed in the future
    // Get renderer object
    DLNARenderer *renderer = qobject_cast<DLNARenderer *>(sender());
    qDebug() << "^Detected response type: "+responseType;

    if (m_queue.isEmpty())
        return;

    // Handle responses
    if(responseType == "StopResponse")
    {
        // Host file, and send its url to renderer
        int id = m_fileServer->serveFile(QUrl(m_queue[0])); // File to serve

        QString fileName = m_fileServer->getFilenameFromID(id),
                localAddress = getLocalAddress().toString(),
                portNumber = QString::number(port);

        renderer->setPlaybackUrl(QUrl(QString("http://%1:%2/%3/%4").arg(localAddress, portNumber,QString::number(id), fileName)),
                                 QFileInfo(m_queue[0]));

    }
    else if(responseType == "SetAVTransportURIResponse")
        renderer->playPlayback(); // Just play the playback url
}


