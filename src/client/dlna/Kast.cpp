#include "Kast.h"
#include "SSDPDiscovery.h"
#include "DLNAPlaybackInfo.h"
#include <QDebug>

Kast::Kast(QObject *parent) : QObject(parent)
{
    SSDPdiscovery *discovery = new SSDPdiscovery(this);
    connect(discovery, SIGNAL(foundRenderer(DLNARenderer*)), this, SLOT(onFoundRenderer(DLNARenderer*)));
    // Start SSDP discovery
    discovery->run();
}

void Kast::onFoundRenderer(DLNARenderer *renderer)
{
    qDebug() << "Renderer found: " + renderer->getName();
    connect(renderer, SIGNAL(receivedResponse(QString,QString)), this, SLOT(onHttpResponse(QString,QString)));
    // Stop playback. Responses will be handled in handleResponse
    //renderer->stopPlayback();
    m_renderers[renderer->getName()] = renderer;
}

QStringList Kast::getRenderers()
{
    return m_renderers.keys();
}

DLNARenderer *Kast::renderer(const QString &name)
{
    auto it = m_renderers.find(name);
    if (m_renderers.end() != it)
    {
        return *it;
    }
    return nullptr;
}

void Kast::onHttpResponse(const QString& responseType, const QString& data)
{
    Q_UNUSED(data); //May be needed in the future
    // Get renderer object
    DLNARenderer *renderer = qobject_cast<DLNARenderer *>(sender());
    qDebug() << "^Detected response type: "+responseType;

    // Handle responses
    if (responseType == "SetAVTransportURIResponse")
    {
        // Just play the playback url
        renderer->playPlayback();
    }
    else if (responseType == "StopResponse")
    {

    }
    else if (responseType == "PlayResponse")
    {

    }
    else if (responseType == "SeekResponse")
    {

    }
}


