#include "SSDPDiscovery.h"
#include "config.h"

// DLNA renderers discovery class
SSDPdiscovery::SSDPdiscovery(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    connect(m_nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(processData(QNetworkReply*)));
}

SSDPdiscovery::~SSDPdiscovery()
{
    m_multicastUdpSocket->close();
    delete m_multicastUdpSocket;
}

// Starts SSDP discovery. Dont call it, when you have IP adress specified, just procced to ->findRendererFromUrl
void SSDPdiscovery::run()
{
    // DLNA discovery adress
    QHostAddress groupAddress = QHostAddress("239.255.255.250");

    m_multicastUdpSocket = new QUdpSocket(this);
    Config cfg;
    QString ip = cfg.read<QString>("dlnaUseIP");
    m_multicastUdpSocket->bind((ip.isEmpty() ? QHostAddress::AnyIPv4 : QHostAddress(ip)), 1901, QUdpSocket::ShareAddress);
    m_multicastUdpSocket->joinMulticastGroup(groupAddress);

    connect(m_multicastUdpSocket, SIGNAL(readyRead()),
            this, SLOT(processPendingDatagrams()));

    QByteArray datagram;
    // Multicast message, used for discovery of DLNA devices
    datagram.append( "M-SEARCH * HTTP/1.1\r\n" );
    datagram.append( "HOST: 239.255.255.250:1900\r\n" );
    datagram.append( "MX: 5\nMan: \"ssdp:discover\"\r\n" );
    datagram.append( "ST: urn:schemas-upnp-org:device:MediaRenderer:1\r\n\r\n" );

    m_multicastUdpSocket->writeDatagram(datagram.data(), datagram.size(), groupAddress, 1900);
}

QList<DLNARenderer *> &SSDPdiscovery::renderers()
{
    return m_knownRenderers;
}

// Process received datagrams, and get IP of DLNA DMR
void SSDPdiscovery::processPendingDatagrams()
{
    while(m_multicastUdpSocket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = m_multicastUdpSocket->receiveDatagram();
        QStringList list = QString(datagram.data()).split("\r\n");
        auto it = std::find_if(list.begin(), list.end(), [](const QString& i) {
            return i.startsWith("location:", Qt::CaseInsensitive);
        });
        if (list.end() != it)
            findRendererFromUrl(QUrl(it->mid(10).simplified()));
    }
}

void SSDPdiscovery::findRendererFromUrl(const QUrl & url)
{
    // Query renderer info
    m_nam->get(QNetworkRequest(url));
}

// This function process request data, into DLNA Renderer
void SSDPdiscovery::processData(QNetworkReply *reply)
{
    QXmlStreamReader xml(reply->readAll());
    if(!m_knownURLs.contains(reply->url().toString()))
    {
        // Construct renderer object
        DLNARenderer *renderer = new DLNARenderer(reply->url(), this);

        // Parse return url
        while(!xml.hasError() && !xml.atEnd())
        {
            xml.readNextStartElement();
            if(xml.name()=="serviceId" && xml.readElementText()=="urn:upnp-org:serviceId:AVTransport")
            {
                while(xml.name()!="controlURL" && !xml.atEnd())
                    xml.readNextStartElement();
                if(xml.name()=="controlURL")
                    renderer->setControlUrl(xml.readElementText());
            }
            else if(xml.name()=="friendlyName")
                renderer->setName(xml.readElementText());
            // Parse icon list
            else if(xml.name()=="icon" && !xml.isEndElement())
            {
                xml.readNextStartElement();
                DLNARendererIcon icon;
                while(xml.name()!="icon")
                {
                    if(xml.name()=="mimetype") icon.mimetype = xml.readElementText();
                    else if(xml.name()=="width") icon.width = xml.readElementText().toInt();
                    else if(xml.name()=="height") icon.height = xml.readElementText().toInt();
                    else if(xml.name()=="url") icon.url = xml.readElementText();
                    xml.readNextStartElement();
                }
                // Choose the largest icon
                if(renderer->m_icon.width < icon.width)
                    renderer->m_icon = icon;
            }
        }

        m_knownURLs.insert(renderer->getUrl().toString());
        m_knownRenderers.append(renderer);
        emit foundRenderer(renderer);
    }

    reply->close();
    reply->deleteLater();
}
