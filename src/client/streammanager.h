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

    void startDownload(const QStringList& streams);
    void stopDownload();
    void serve(const QString& addr);
    void shutdown();
signals:

public slots:

private:
    QList<StreamReplyPtr> m_streams;
    QNetworkAccessManager* m_nam;
};

#endif // STREAMMANAGER_H
