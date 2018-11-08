#include "SOAPActionManager.h"
#include "DLNAPlaybackInfo.h"
#include "MimeGuesser.h"
#include <QDebug>

// Xml request's body
const QString SOAPXmlHeader = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?>"
                              "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" "
                              "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body><u:",
              SOAPXmlInstanceId = " xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</InstanceID>",
              SOAPXmlActions = "</u:",
              SOAPXmlFooter = "></s:Body></s:Envelope>",
              DIDLLiteString = "&lt;DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\""
                               " xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\""
                               " xmlns:dc=\"http://purl.org/dc/elements/1.1/\""
                               " xmlns:sec=\"http://www.sec.co.kr/\"&gt;&lt;item id=\"f-0\" parentID=\"0\" restricted=\"0\"&gt;"
                               "&lt;dc:title&gt;%1&lt;/dc:title&gt;"
                               "&lt;dc:creator&gt;%2&lt;/dc:creator&gt;"
                               "&lt;upnp:class&gt;object.item.%3Item&lt;/upnp:class&gt;"
                               "&lt;res protocolInfo=\"http-get:*:%4:DLNA.ORG_OP=01;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000\" &gt;"
                               "%5&lt;/res&gt;&lt;/item&gt;&lt;/DIDL-Lite&gt;";


SOAPActionManager::SOAPActionManager(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

void SOAPActionManager::doAction(const QString &action, const QMap<QString, QString> &dataMap, const QUrl &controlUrl)
{
    QNetworkRequest request;
    // Build xml data string
    QString actionData;
    if (!dataMap.isEmpty() || !dataMap.isDetached())
    {
        for (auto it = dataMap.keyBegin(); dataMap.keyEnd() != it; ++it)
        {
            actionData.append("<" + *it + ">");
            actionData.append(dataMap.value(*it));
            actionData.append("</" + *it + ">");
        }
    }

    // Build xml request body
    QString data = SOAPXmlHeader + action + SOAPXmlInstanceId + actionData + SOAPXmlActions + action + SOAPXmlFooter;
    QByteArray actionHeader = QString("\"urn:schemas-upnp-org:service:AVTransport:1#" + action + "\"").toUtf8();
    // Set needed headers
    request.setUrl(controlUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; charset=utf-8");
    request.setRawHeader("SOAPaction", actionHeader);
    // To do not duplicate code, just check, is this action with needed data processing, or not.
    // If you want to add new action, which needs data processing, do it here.

    if (action == "GetPositionInfo")
        connect(m_nam, &QNetworkAccessManager::finished, this, &SOAPActionManager::processPlaybackInfo);
    else
        connect(m_nam, &QNetworkAccessManager::finished, this, &SOAPActionManager::processData);

    m_nam->post(request, data.toUtf8());
}

void SOAPActionManager::processData(QNetworkReply *reply)
{
    QString data = reply->readAll();
    reply->close();
    reply->deleteLater();

    // We want to be able to connect it to few slots, so lets disconnect it for now
    disconnect(m_nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(processData(QNetworkReply*)));

    qDebug() << "Got XML response:" << data;
    // Initial value, used if response type is not detected
    QString responseType = "UNDEFINED";

    // @TODO Add nice error handling here. It should be simple to implement, and could be also handled in receivedResponse signal.
    QXmlStreamReader xml(data);
    while(!xml.hasError() && !xml.atEnd() && !xml.name().contains("Response"))
        xml.readNextStartElement();
    if(xml.name().contains("Response"))
        responseType = xml.name().toString();
    // Emit signal with response's type, and raw data
    emit receivedResponse(responseType, data);

}

void SOAPActionManager::processPlaybackInfo(QNetworkReply *reply)
{
    // Construct xml parser, from reply's data, close socket
    QXmlStreamReader xml(reply->readAll());
    reply->close();
    reply->deleteLater();

    // We want to be able to connect it to few slots, so lets disconnect it for now
    disconnect(m_nam, &QNetworkAccessManager::finished, this, &SOAPActionManager::processPlaybackInfo);

    DLNAPlaybackInfo playbackInfo;
    // Parse return url
    while(!xml.hasError() && !xml.atEnd())
    {
        xml.readNextStartElement();
        if(xml.name() == "RelTime")
            playbackInfo.relTime = QTime::fromString(xml.readElementText(), "hh:mm:ss");
        else if(xml.name() == "TrackDuration")
            playbackInfo.trackDuration = QTime::fromString(xml.readElementText(), "hh:mm:ss");
    }

    emit receivePlaybackInfo(&playbackInfo);
}

// Generates DIDL-Lite metadata
QString SOAPActionManager::generateMetadata(const QFileInfo &fileInfo, const QString &address)
{
    MimeGuesser mg;
    // Construct DIDL-Lite
    return DIDLLiteString.arg(fileInfo.fileName(), fileInfo.owner(), mg.getMediaType(fileInfo.filePath()),
                              mg.fileMimeType(fileInfo), address);
}
