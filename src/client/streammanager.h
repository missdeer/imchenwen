#ifndef STREAMMANAGER_H
#define STREAMMANAGER_H

#include <QObject>
#include "streamreply.h"

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
QT_END_NAMESPACE

class QHttpRequest;
class QHttpResponse;
class QHttpServer;

class StreamManager : public QObject
{
    Q_OBJECT
public:
    explicit StreamManager(QNetworkAccessManager* nam, QObject *parent = 0);
    ~StreamManager();
    void startDownload(const QStringList& streams);
    void stopDownload();
    void serve(const QString& addr);
    void shutdown();
    const QStringList& urls();
signals:

private slots:
    void finished();
    void handle(QHttpRequest *req, QHttpResponse *resp);
private:
    QList<StreamReplyPtr> m_streams;
    QNetworkAccessManager* m_nam;
    QHttpServer *m_server;
    QStringList m_localUrls;
    QStringList m_remoteUrls;
    int m_downloadIndex;
    void download(int i);
};

#endif // STREAMMANAGER_H
