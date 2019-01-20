#include "outofchinamainlandproxyfactory.h"
#include "config.h"
#include "browser.h"
#include "networkreplyhelper.h"

OutOfChinaMainlandProxyFactory::OutOfChinaMainlandProxyFactory()
{

}

void OutOfChinaMainlandProxyFactory::init()
{
    // download China domain list first
    Config cfg;
    QNetworkRequest req;
    QUrl u(cfg.read(QLatin1String("chinaDomain"), QString("https://cdn.jsdelivr.net/gh/felixonmars/dnsmasq-china-list/accelerated-domains.china.conf")));
    req.setUrl(u);
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QNetworkAccessManager &nam = Browser::instance().networkAccessManager();
    auto reply = nam.get(req);
    auto helper = new NetworkReplyHelper(reply);
    connect(helper, &NetworkReplyHelper::done, this, &OutOfChinaMainlandProxyFactory::done);
}

void OutOfChinaMainlandProxyFactory::updateProxyCache(const QNetworkProxy &proxy)
{
    m_proxyCache = proxy;
}

QList<QNetworkProxy> OutOfChinaMainlandProxyFactory::queryProxy(const QNetworkProxyQuery &query)
{
    QList<QNetworkProxy> m_proxyList;
    if (needProxy(query.url().toString()))
        m_proxyList.append(m_proxyCache);
    else
        m_proxyList.append(QNetworkProxy::NoProxy);

    return m_proxyList;
}

bool OutOfChinaMainlandProxyFactory::needProxy(const QString &url)
{
    if (url.isEmpty())
        return true;
    QString hostAddr = QUrl(url).host();
    QHostAddress host(hostAddr);
    if (!host.isNull()
        || host.isInSubnet(QHostAddress("127.0.0.0"), 8)
        || host.isInSubnet(QHostAddress("192.168.0.0"), 16)
        || host.isInSubnet(QHostAddress("169.254.0.0"), 16)
        || host.isInSubnet(QHostAddress("224.0.0.0"), 4)
        || host.isInSubnet(QHostAddress("240.0.0.0"), 4)
        || host.isInSubnet(QHostAddress("10.0.0.0"), 8)
        || host.isInSubnet(QHostAddress("172.16.0.0"), 12))
        return false;
    auto hostSegs = hostAddr.split('.');
    while (!hostSegs.isEmpty() && m_chinaDomains.find(hostSegs.join('.')) == m_chinaDomains.end())
    {
        hostSegs.removeFirst();
    }
    return hostSegs.isEmpty();  // not found, so it's out of China Mainland
}

void OutOfChinaMainlandProxyFactory::done()
{
    NetworkReplyHelper* helper = qobject_cast<NetworkReplyHelper*>(sender());
    helper->deleteLater();

    QByteArray content = helper->content();
    auto lines = content.split('\n');
    for (const auto& line : lines)
    {
        if (line.startsWith('#'))
            continue;

        auto segs = line.split('/');
        if (segs.length() != 3)
            continue;
        m_chinaDomains.insert(segs.at(1), true);
    }
}
