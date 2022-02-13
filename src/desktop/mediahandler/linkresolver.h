#ifndef LINKRESOLVER_H
#define LINKRESOLVER_H

#include <QObject>
#include <QSharedPointer>
#include <QTime>
#include <QJsonValue>
#include "luxprocess.h"
#include "yougetprocess.h"
#include "youtubedlprocess.h"
#include "ykdlprocess.h"

class LinkResolver : public QObject
{
    Q_OBJECT

    struct Resolver {
        LinkResolverProcess *process;
        Streams *streams;
    };

public:
    explicit LinkResolver(QObject *parent = nullptr);
    ~LinkResolver();
    void resolve(const QString &url);
    void setupResolvers();
    void terminateResolvers();
signals:
    void done(const QString& url, MediaInfoPtr);
    void error(const QString& url, const QString &msg);
public slots:

private slots:
    void onReadResolverOutput(const QByteArray& data);
private:
    bool m_stopped;
    QString m_lastUrl;
    MediaInfoPtr m_mediaInfo;
    YouGetProcess m_yougetProcess;
    YKDLProcess m_ykdlProcess;
    YoutubeDLProcess m_youtubedlProcess;
    LuxProcess m_luxProcess;
};

#endif // LINKRESOLVER_H
