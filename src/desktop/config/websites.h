#ifndef WEBSITES_H
#define WEBSITES_H

#include <QList>
#include <QSharedPointer>
#include <QDomElement>
#include <QNetworkReply>
#include <QSslError>

struct Website
{
    QString title;
    QString url;
    QString category;
    bool favourite;
};

typedef QSharedPointer<Website> WebsitePtr;

typedef QList<WebsitePtr> WebsiteList;

class Websites : public QObject
{
    Q_OBJECT
public:
    explicit Websites(QObject *parent = nullptr);
    void update(bool withoutUI = false);

    bool isIn(const QUrl &url, const QString &category = QString());
    const QString& findURL(const QString& name);
    WebsiteList favourites();
    WebsiteList inChina();
    WebsiteList abroad();
    WebsiteList onlineFilm();
signals:
    void ready();
private slots:
    void onNetworkError(QNetworkReply::NetworkError code);
    void onNetworkSSLErrors(const QList<QSslError> &errors);
    void onReadyRead();
    void onReadFinished();

private:
    WebsiteList m_websites;
    QByteArray m_data;
    void parseWebsiteNode(QDomElement website, const QString &category);
    void doParse();
};

#endif // WEBSITES_H
