#include "vipresolver.h"
#include "browser.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>

VIPResolver::VIPResolver(QObject *parent)
    : QObject(parent)
{

}

void VIPResolver::update()
{
    m_data.clear();
    QNetworkRequest req;
    QUrl u("https://gist.githubusercontent.com/missdeer/1a841346e123a9d21337d0b8d5d546c1/raw/vip.txt");
    req.setUrl(u);
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QNetworkAccessManager &nam = Browser::instance().networkAccessManager();
    QNetworkReply *reply = nam.get(req);
    connect(reply, &QIODevice::readyRead, this, &VIPResolver::onReadyRead);
    connect(reply, &QNetworkReply::finished, this, &VIPResolver::onReadFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &VIPResolver::onNetworkError);
    connect(reply, &QNetworkReply::sslErrors, this, &VIPResolver::onNetworkSSLErrors);
}

void VIPResolver::resolve(const QString &url)
{

}

void VIPResolver::onNetworkError(QNetworkReply::NetworkError code)
{
    qWarning() << code;
}

void VIPResolver::onNetworkSSLErrors(const QList<QSslError> &errors)
{
    for (auto & e : errors)
        qWarning() << e.errorString();
}

void VIPResolver::onReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    m_data.append( reply->readAll());
}

void VIPResolver::onReadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();
    onReadyRead();
    auto lines = m_data.split('\n');
    m_resolvers.clear();
    for (const auto & line : lines)
    {
        m_resolvers.append(QString(line.trimmed()));
    }
}
