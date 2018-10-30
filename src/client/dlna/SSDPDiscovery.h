#ifndef SSDPDISCOVERY_H
#define SSDPDISCOVERY_H

#include <QtNetwork>
#include <DLNARenderer.h>
#include <QSet>
#include <QList>

class SSDPdiscovery : public QObject
{
    Q_OBJECT
public:
    explicit SSDPdiscovery(QObject *parent = nullptr);
    void findRendererFromUrl(const QUrl & url);
    void run();
private slots:
    void processData(QNetworkReply*);
    void processPendingDatagrams();
private:
    QNetworkAccessManager *m_nam;
    QUdpSocket *m_multicastUdpSocket;
    QSet<QString> m_knownURLs;
    QList<DLNARenderer*> m_knownRenderers;
signals:
    void foundRenderer(DLNARenderer *renderer);
};

#endif // SSDPDISCOVERY_H
