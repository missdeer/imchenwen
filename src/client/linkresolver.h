#ifndef LINKRESOLVER_H
#define LINKRESOLVER_H

#include <QObject>
#include <QNetworkReply>
#include <QSharedPointer>

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
    QString site;
    QString title;
    Streams ykdl;
    Streams you_get;
    Streams youtube_dl;
};

typedef QSharedPointer<MediaInfo> MediaInfoPtr;

struct HistoryItem
{
    QString url;
    QTime time;
    MediaInfoPtr mi;
};

typedef QSharedPointer<HistoryItem> HistoryItemPtr;

class LinkResolver : public QObject
{
    Q_OBJECT
public:
    explicit LinkResolver(QObject *parent = 0);
    void resolve(const QUrl& url, bool silent = false);
signals:
    void resolvingFinished(MediaInfoPtr);
    void resolvingError();
    void resolvingSilentFinished(MediaInfoPtr);
    void resolvingSilentError();
public slots:

private slots:
    void error(QNetworkReply::NetworkError code);
    void finished();
    void sslErrors(const QList<QSslError> & errors);
    void readyRead();

private:
    QByteArray m_content;
    QList<HistoryItemPtr> m_history;
    void parseNode(const QJsonObject& o, MediaInfoPtr mi, Streams& streams);
};

#endif // LINKRESOLVER_H
