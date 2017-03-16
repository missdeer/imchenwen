#ifndef LINKRESOLVER_H
#define LINKRESOLVER_H

#include <QObject>
#include <QNetworkReply>

struct StreamInfo
{
    QString container;
    QString quality;
    QStringList urls;
};

typedef QList<StreamInfo> Streams;

struct MediaInfo
{
    QString site;
    QString title;
    Streams preferred;
    Streams backup;
};

class LinkResolver : public QObject
{
    Q_OBJECT
public:
    explicit LinkResolver(QObject *parent = 0);
    void resolve(const QUrl& url);
signals:
    void resolvingFinished(const MediaInfo&);
    void resolvingError();
public slots:

private slots:
    void error(QNetworkReply::NetworkError code);
    void finished();
    void sslErrors(const QList<QSslError> & errors);
    void readyRead();

private:
    QByteArray m_content;
};

#endif // LINKRESOLVER_H
