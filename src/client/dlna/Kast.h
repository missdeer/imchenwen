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
    void addItemToQueue(const QString & item_url);
    QStringList getRenderers();
    void setPlaybackUrl(const QString & renderer, const QUrl & url, const QFileInfo & fileInfo);
    void setNextPlaybackUrl(const QString & renderer, const QUrl & url);
    void play(const QString & renderer);
    void pause(const QString & renderer);
    void stop(const QString & renderer);
    void resume(const QString & renderer);
    void seekPlayback(const QString & renderer, QTime time);
signals:

private slots:
    void onFoundRenderer(DLNARenderer*);
    void onHttpResponse(const QString, const QString);
private:
    HttpFileServer *m_fileServer;
    QStringList m_queue;
    QMap<QString, DLNARenderer *> m_renderers;
    QHostAddress getLocalAddress();
};
#endif // KAST_H
