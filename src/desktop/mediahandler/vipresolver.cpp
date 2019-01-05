#include "vipresolver.h"
#include "browser.h"
#include "sniffer.h"
#include "config.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>

VIPResolver::VIPResolver(QObject *parent)
    : QObject(parent)
    , m_ready(false)
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
    m_resolvers.clear();
    m_data.clear();
    QNetworkRequest req;
    Config cfg;
    QUrl u(cfg.read<QString>(QLatin1String("vipResolvers")));
    req.setUrl(u);
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QNetworkAccessManager &nam = Browser::instance().networkAccessManager();
    QNetworkReply *reply = nam.get(req);
    connect(reply, &QIODevice::readyRead, this, &VIPResolver::onReadyRead);
    connect(reply, &QNetworkReply::finished, this, &VIPResolver::onReadFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &VIPResolver::onNetworkError);
    connect(reply, &QNetworkReply::sslErrors, this, &VIPResolver::onNetworkSSLErrors);
}

bool VIPResolver::doSniff(Sniffer *sniffer)
{
    if (m_resolverIndex >= m_resolvers.length())
        return false;
    QString u = m_resolvers.at(m_resolverIndex) + m_lastResolveUrl;
    sniffer->sniff(u);
    m_resolverIndex++;
    return true;
}

void VIPResolver::resolve(const QString &url)
{
    if (url == m_lastResolveUrl)
    {
        emit done(url, m_results);
        return;
    }

    stop();
    m_stopped = false;
    m_finishedCount = 0;
    m_resolverIndex = 0;
    m_lastResolveUrl = url;
    m_results.clear();
    for (auto sniffer : m_sniffers)
    {
        if (!doSniff(sniffer))
            break;
    }
}

void VIPResolver::stop()
{
    m_stopped = true;
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
    m_ready = !m_resolvers.isEmpty();
}

void VIPResolver::continueSniff(Sniffer *sniffer)
{
    if (!doSniff(sniffer) && m_finishedCount == m_resolvers.length())
    {
        if (m_results.isEmpty())
            emit error();
        else if (!m_stopped)
            emit done(m_lastResolveUrl, m_results);
    }
}

void VIPResolver::onSnifferDone(const QString &originalUrl, const QString &result)
{
    m_finishedCount++;
    if (m_stopped)
        return;

    if (!m_results.contains(result))
    {
        m_results.append(result);
        if (!m_stopped)
            emit done(m_lastResolveUrl, m_results);
    }

    Sniffer* sniffer = qobject_cast<Sniffer*>(sender());
    continueSniff(sniffer);
}

void VIPResolver::onSnifferError()
{
    m_finishedCount++;
    if (m_stopped)
        return;
    Sniffer* sniffer = qobject_cast<Sniffer*>(sender());
    continueSniff(sniffer);
}
