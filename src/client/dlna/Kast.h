#ifndef KAST_H
#define KAST_H

#include "DLNARenderer.h"
#include "HTTPFileServer.h"
#include "DLNAPlaybackInfo.h"

#include <QtNetwork>


class Kast : public QObject
{
    Q_OBJECT
public:
    Kast(QObject *parent = nullptr);
    void addItemToQueue(QString & item_url);
    QHostAddress getLocalAddress();
private:
    HttpFileServer *m_fileServer;
    QStringList m_queue;
private slots:
    void onFoundRenderer(DLNARenderer*);
    void onHttpResponse(const QString, const QString);
};
#endif // KAST_H
