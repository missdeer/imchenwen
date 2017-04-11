#ifndef STREAMMANAGER_H
#define STREAMMANAGER_H

#include <QObject>
#include "streamreply.h"

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
QT_END_NAMESPACE

class StreamManager : public QObject
{
    Q_OBJECT
public:
    explicit StreamManager(QNetworkAccessManager* nam, QObject *parent = 0);
    ~StreamManager();
    void startDownload(const QStringList& streams);
    void stopDownload();
    const QStringList& urls();
signals:
    void readyRead();
    void cancelRead();
private slots:
    void finished();
    void onLocalReadyRead();
private:
    QList<StreamReplyPtr> m_streams;
    QNetworkAccessManager* m_nam;
    QStringList m_localUrls;
    QStringList m_remoteUrls;
    int m_downloadIndex;
    int m_finishedCount;
    int m_runningCount;
    void download(int i);
};

#endif // STREAMMANAGER_H
