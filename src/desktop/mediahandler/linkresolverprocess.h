#ifndef LINKRESOLVERPROCESS_H
#define LINKRESOLVERPROCESS_H

#include <QProcess>
#include <QTime>
#include <QSharedPointer>
#include <QJsonValue>

struct StreamInfo
{
    QString container;
    QString quality;
    QStringList urls;
};

typedef QSharedPointer<StreamInfo> StreamInfoPtr;
typedef QList<StreamInfoPtr> Streams;

struct Subtitle
{
    QString language;
    QString url;
    bool manual;
};

typedef QSharedPointer<Subtitle> SubtitlePtr;
typedef QList<SubtitlePtr> Subtitles;

struct MediaInfo
{
    QString url;
    QString site;
    QString title;
    Streams ykdl;
    Streams you_get;
    Streams youtube_dl;
    Streams annie;
    Subtitles subtitles;
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

class LinkResolverProcess : public QObject
{
    Q_OBJECT
public:
    explicit LinkResolverProcess(QObject *parent = nullptr);
    virtual ~LinkResolverProcess();
    virtual void setProgram(const QString& program);
    virtual void start(const QString& url);
    virtual void setTimeout(int timeout);
    virtual void parseNode(const QJsonObject&, MediaInfoPtr);
    virtual void init();
signals:
    void done(const QByteArray&);
public slots:
    void stop();
private slots:
    void onReadStandardOutput();
    void onFinished(int, QProcess::ExitStatus);
protected:
    QProcess m_process;
    QByteArray m_data;
    QStringList m_args;
    int m_timeout;
};

#endif // LINKRESOLVERPROCESS_H
