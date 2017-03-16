#include "linkresolver.h"
#include <QNetworkRequest>
#include <QNetworkAccessManager>

static QNetworkAccessManager nam;

LinkResolver::LinkResolver(QObject *parent)
    : QObject(parent)
{
}

void LinkResolver::resolve(const QUrl &url)
{
    m_content.clear();

    QNetworkRequest req(QUrl("https://pjp.xyying.me/v1/parse"));
    QByteArray data;
    data.append("apikey=yb2Q1ozScRfJJ");
    data.append("&url=");
    data.append(url.toString().toUtf8().toPercentEncoding());
    QNetworkReply *reply = nam.post(req, data);

    connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(error(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(sslErrors(QList<QSslError>)));
    connect(reply, SIGNAL(finished()),
            this, SLOT(finished()));
}

void LinkResolver::error(QNetworkReply::NetworkError code)
{
    qDebug() << "resolving error:" << code;
    emit resolvingError();
}

void LinkResolver::finished()
{
    MediaInfo mi;
    emit resolvingFinished(mi);
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();
}

void LinkResolver::sslErrors(const QList<QSslError> &errors)
{
    for (auto e : errors)
        qDebug() << "resovling ssl errors:" << e.errorString();
    emit resolvingError();
}

void LinkResolver::readyRead()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 200 && statusCode < 300) {
        m_content.append( reply->readAll());
    }
}
