#include "vipresolver.h"
#include "browser.h"
#include "sniffer.h"
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
    m_data.clear();
    QNetworkRequest req;
    QUrl u("https://gist.githubusercontent.com/missdeer/ce589e84b4101e90293f15d4c7aa2354/raw/vip.txt");
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
    stop();
    m_done = false;
    m_finishedCount = 0;
    m_resolverIndex = 0;
    m_lastResolveUrl = url;
    for (auto sniffer : m_sniffers)
    {
        if (!doSniff(sniffer))
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
    m_ready = !m_resolvers.isEmpty();
}

void VIPResolver::continueSniff(Sniffer *sniffer)
{
    if (!doSniff(sniffer) && m_finishedCount == m_resolvers.length())
    {
        if (m_results.isEmpty())
            emit error();
        else
            emit done(m_results);
    }
}

void VIPResolver::onSnifferDone(const QString &res)
{
    m_finishedCount++;
    if (m_done)
        return;
    if (!m_results.contains(res))
        m_results.append(res);
    if (m_results.length() > 1)
    {
        m_done = true;
        emit done(m_results);
        return;
    }
    Sniffer* sniffer = qobject_cast<Sniffer*>(sender());
    continueSniff(sniffer);
}

void VIPResolver::onSnifferError()
{
    m_finishedCount++;
    if (m_done)
        return;
    Sniffer* sniffer = qobject_cast<Sniffer*>(sender());
    continueSniff(sniffer);
}
