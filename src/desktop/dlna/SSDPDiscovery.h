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
    ~SSDPdiscovery() override;
    void findRendererFromUrl(const QUrl & url);
    void run();
    QList<DLNARenderer *> &renderers();
private slots:
    void processData();
    void processPendingDatagrams();
private:
    QUdpSocket *m_multicastUdpSocket;
    QSet<QString> m_knownURLs;
    QList<DLNARenderer*> m_knownRenderers;
signals:
    void foundRenderer(DLNARenderer *renderer);
};

#endif // SSDPDISCOVERY_H
