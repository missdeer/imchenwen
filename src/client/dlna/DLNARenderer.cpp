#include "DLNARenderer.h"
#include "DLNAPlaybackInfo.h"

#include <QDebug>

DLNARenderer::DLNARenderer(QUrl url, QObject *parent)
    : QObject(parent)
    , m_sam(new SOAPActionManager())
    , m_serverUrl(url)
{
    // Pass SOAPActionManager's signals to parent class
    connect(m_sam, SIGNAL(receivePlaybackInfo(DLNAPlaybackInfo*)), this, SIGNAL(receivePlaybackInfo(DLNAPlaybackInfo*)));
    connect(m_sam, SIGNAL(receivedResponse(const QString,const QString)), this, SIGNAL(receivedResponse(const QString, const QString)));
}

QString DLNARenderer::getName()
{
    return m_serverName;
}

QUrl DLNARenderer::getUrl()
{
    return m_serverUrl;
}

void DLNARenderer::setName(const QString & name)
{
    m_serverName = name;
}

void DLNARenderer::setControlUrl(const QString & url)
{
    m_fullcontrolUrl = m_serverUrl;
    m_fullcontrolUrl.setPath(url);
}

void DLNARenderer::setPlaybackUrl(const QUrl & url, const QFileInfo &fileInfo)
{
    QMap<QString, QString> dataMap;
    // Add URI to request, URL-encode it
    QString urlString = url.toString().replace(url.fileName(), QString(QUrl::toPercentEncoding(url.fileName())));
    dataMap.insert("CurrentURI", urlString);
    dataMap.insert("CurrentURIMetaData",m_sam->generateMetadata(fileInfo, urlString));
    m_sam->doAction(
                "SetAVTransportURI", // Action
                dataMap,  // Action Data
                m_fullcontrolUrl); // Control url
}

void DLNARenderer::setNextPlaybackUrl(const QUrl & url)
{
    QMap<QString, QString> dataMap;
    // Add URI to request, URL-encode it
    QString urlString = url.toString().replace(url.fileName(), QString(QUrl::toPercentEncoding(url.fileName())));
    dataMap.insert("NextURI", urlString);
    dataMap.insert("NextURIMetaData", "");
    m_sam->doAction("SetNextAVTransportURI", dataMap, m_fullcontrolUrl);
}

void DLNARenderer::queryPlaybackInfo()
{
    m_sam->doAction("GetPositionInfo", QMap<QString, QString>(), m_fullcontrolUrl);
}

void DLNARenderer::playPlayback()
{
    QMap<QString, QString> dataMap;
    dataMap.insert("Speed", "1");
    m_sam->doAction("Play", dataMap, m_fullcontrolUrl);
}

void DLNARenderer::pausePlayback()
{
    m_sam->doAction("Pause", QMap<QString, QString>(), m_fullcontrolUrl);
}

void DLNARenderer::stopPlayback()
{
    m_sam->doAction("Stop", QMap<QString, QString>(), m_fullcontrolUrl);
}

void DLNARenderer::seekPlayback(QTime time)
{
    QMap<QString, QString> dataMap;
    dataMap.insert("InstanceID", "0");
    dataMap.insert("Unit", "REL_TIME");
    dataMap.insert("Target", time.toString());
    m_sam->doAction("Seek", dataMap, m_fullcontrolUrl);
}

void DLNARenderer::previousItem()
{
    m_sam->doAction("Previous", QMap<QString, QString>(), m_fullcontrolUrl);
}

void DLNARenderer::nextItem()
{
    m_sam->doAction("Next", QMap<QString, QString>(), m_fullcontrolUrl);
}
