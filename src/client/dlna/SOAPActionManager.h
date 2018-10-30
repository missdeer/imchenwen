#ifndef SOAPACTIONMANAGER_H
#define SOAPACTIONMANAGER_H
#include "DLNAPlaybackInfo.h"
#include <QObject>
#include <QtNetwork>


// This class handles network requests from DLNARenderer.
class SOAPActionManager : public QObject
{
    Q_OBJECT
public:
    explicit SOAPActionManager(QObject *parent = nullptr);
    void doAction(const QString &action, const QMap<QString, QString> &dataMap, const QUrl &controlUrl);
    QString generateMetadata(const QFileInfo &fileInfo, const QString & address);
signals:
    void receivePlaybackInfo(DLNAPlaybackInfo*);
    void receivedResponse(const QString, const QString);
private:
    QNetworkAccessManager *m_nam;
private slots:
    void processData(QNetworkReply *reply);
    void processPlaybackInfo(QNetworkReply *reply);
};

#endif // SOAPACTIONMANAGER_H
