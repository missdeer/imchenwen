#ifndef LINKRESOLVER_H
#define LINKRESOLVER_H

#include <QObject>
#include <QSharedPointer>
#include <QTime>
#include "linkresolverprocess.h"

struct StreamInfo
{
    QString container;
    QString quality;
    QStringList urls;
};

typedef QSharedPointer<StreamInfo> StreamInfoPtr;
typedef QList<StreamInfoPtr> Streams;

struct MediaInfo
{
    QString url;
    QString site;
    QString title;
    Streams ykdl;
    Streams you_get;
    Streams youtube_dl;
    Streams annie;
    int resultCount;
};

typedef QSharedPointer<MediaInfo> MediaInfoPtr;

struct HistoryItem
{
    QString url;
    QTime time;
    bool vip;
    MediaInfoPtr mi;
};

typedef QSharedPointer<HistoryItem> HistoryItemPtr;

class LinkResolver : public QObject
{
    Q_OBJECT
public:
    explicit LinkResolver(QObject *parent = nullptr);
    void resolve(const QString & url);
signals:
    void resolvingFinished(MediaInfoPtr);
    void resolvingError(const QString&);
public slots:

private slots:
    void readYouGetOutput(const QByteArray& data);
    void readYKDLOutput(const QByteArray &data);
    void readYoutubeDLOutput(const QByteArray &data);
    void readAnnieOutput(const QByteArray &data);
private:
    QString m_lastUrl;
    LinkResolverProcess m_yougetProcess;
    LinkResolverProcess m_ykdlProcess;
    LinkResolverProcess m_youtubedlProcess;
    LinkResolverProcess m_annieProcess;
    MediaInfoPtr m_mediaInfo;
    void parseNode(const QJsonObject& o, MediaInfoPtr mi, Streams& streams);
    void parseYouGetNode(const QJsonObject& o, MediaInfoPtr mi, Streams& streams);
    void parseYKDLNode(const QJsonObject& o, MediaInfoPtr mi, Streams& streams);
    void parseYoutubeDLNode(const QJsonObject& o, MediaInfoPtr mi, Streams& streams);
    void parseAnnieNode(const QJsonObject& o, MediaInfoPtr mi, Streams& streams);
    void resolveByYouGet(const QString &url);
    void resolveByYKDL(const QString& url);
    void resolveByYoutubeDL(const QString &url);
    void resolveByAnnie(const QString& url);
    void submitResolveResult();
};

#endif // LINKRESOLVER_H
