#ifndef DLNARENDERER_H
#define DLNARENDERER_H

#include "SOAPActionManager.h"
#include "DLNAPlaybackInfo.h"
#include "DLNARendererIcon.h"

#include <QtNetwork>

class DLNARenderer : public QObject
{
    Q_OBJECT
public:
    explicit DLNARenderer(QUrl url, QObject *parent = nullptr);
    
    QUrl getUrl();
    QString getControlUrl();
    QString getName();
    DLNARendererIcon m_icon;
    
    void setName(const QString & name);
    void setControlUrl(const QString & name);

    // DLNA functions
    void setPlaybackUrl(const QUrl & url, const QFileInfo & fileInfo);
    void setNextPlaybackUrl(const QUrl & url);

    void playPlayback();
    void stopPlayback();
    void pausePlayback();
    void seekPlayback(QTime time);

    void nextItem();
    void previousItem();
    void queryPlaybackInfo();
private:
    SOAPActionManager *m_sam;
    QUrl m_serverUrl;
    QUrl m_fullcontrolUrl;
    QString m_serverName;
signals:
    void receivePlaybackInfo(DLNAPlaybackInfo*);
    void receivedResponse(const QString, const QString);

};

#endif // DLNARENDERER_H
