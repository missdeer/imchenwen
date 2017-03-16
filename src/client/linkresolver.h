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
    Streams preferred;
    Streams backup;
};

typedef QSharedPointer<MediaInfo> MediaInfoPtr;

class LinkResolver : public QObject
{
    Q_OBJECT
public:
    explicit LinkResolver(QObject *parent = 0);
    void resolve(const QUrl& url);
signals:
    void resolvingFinished(MediaInfoPtr);
    void resolvingError();
public slots:

private slots:
    void error(QNetworkReply::NetworkError code);
    void finished();
    void sslErrors(const QList<QSslError> & errors);
    void readyRead();

private:
    QByteArray m_content;
    void parsePreferredNode(const QJsonObject& o, MediaInfoPtr mi);
    void parseBackupNode(const QJsonObject& o, MediaInfoPtr mi);

    void parseNode(const QJsonObject& o, MediaInfoPtr mi, Streams& streams);
};

#endif // LINKRESOLVER_H
