#include "vipresolver.h"
#include "browser.h"
#include "sniffer.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>

VIPResolver::VIPResolver(QObject *parent)
    : QObject(parent)
{
    m_sniffers << new Sniffer
               << new Sniffer
               << new Sniffer
               << new Sniffer
               << new Sniffer;
    for (auto sniffer : m_sniffers)
    {
        connect(sniffer, &Sniffer::done, this, &VIPResolver::onSnifferDone);
        connect(sniffer, &Sniffer::error, this, &VIPResolver::onSnifferError);
    }
}

VIPResolver::~VIPResolver()
{
    stop();
    qDeleteAll(m_sniffers);
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

bool VIPResolver::doSniff(Sniffer *sniffer, const QString &url)
{
    if (m_resolverIndex >= m_resolvers.length())
        return false;
    QString u = m_results.at(m_resolverIndex) + url;
    sniffer->sniff(u);
    m_resolverIndex++;
    return true;
}

void VIPResolver::resolve(const QString &url)
{
    stop();
    m_resolverIndex = 0;
    m_lastResolveUrl = url;
    for (auto sniffer : m_sniffers)
    {
        if (!doSniff(sniffer, url))
            break;
    }
}

void VIPResolver::stop()
{
    for (auto sniffer : m_sniffers)
        sniffer->stop();
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

void VIPResolver::onSnifferDone(const QString &res)
{
    Sniffer* sniffer = qobject_cast<Sniffer*>(sender());
    if (!m_results.contains(res))
        m_results.append(res);
    if (m_results.length() > 1)
    {
        emit done(m_results);
        return;
    }
    if (!doSniff(sniffer, m_lastResolveUrl))
    {
        if (m_results.isEmpty())
            emit error();
        else
            emit done(m_results);
    }
}

void VIPResolver::onSnifferError()
{
    Sniffer* sniffer = qobject_cast<Sniffer*>(sender());

    if (!doSniff(sniffer, m_lastResolveUrl))
    {
        if (m_results.isEmpty())
            emit error();
        else
            emit done(m_results);
    }
}
