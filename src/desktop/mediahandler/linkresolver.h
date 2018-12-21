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

    struct Resolver {
        QString name;
        LinkResolverProcess *process;
        std::function<void(const QJsonObject , MediaInfoPtr , Streams &)> parse;
        Streams *streams;
        QStringList args;
    };

public:
    explicit LinkResolver(QObject *parent = nullptr);
    ~LinkResolver();
    void resolve(const QString &url);
    void setupResolvers();
signals:
    void done(const QString& url, MediaInfoPtr);
    void error(const QString& url, const QString&msg);
public slots:

private slots:
    void onReadResolverOutput(const QByteArray& data);
private:
    QString m_lastUrl;
    MediaInfoPtr m_mediaInfo;
    LinkResolverProcess m_yougetProcess;
    LinkResolverProcess m_ykdlProcess;
    LinkResolverProcess m_youtubedlProcess;
    LinkResolverProcess m_annieProcess;
    QList<Resolver> m_resolvers;
    void parseYouGetNode(const QJsonObject& o, MediaInfoPtr mi, Streams& streams);
    void parseYKDLNode(const QJsonObject& o, MediaInfoPtr mi, Streams& streams);
    void parseYoutubeDLNode(const QJsonObject& o, MediaInfoPtr mi, Streams& streams);
    void parseAnnieNode(const QJsonObject& o, MediaInfoPtr mi, Streams& streams);
    void terminateResolvers();
};

#endif // LINKRESOLVER_H
