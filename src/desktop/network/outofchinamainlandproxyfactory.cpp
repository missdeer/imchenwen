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

    QString host = query.url().host();
    auto hostSegs = host.split('.');
    DomainMap* domainMap = &m_chinaDomains;
    for (auto it = hostSegs.rbegin(); hostSegs.rend() != it; ++it)
    {
        if (!domainMap || domainMap->find(*it) == domainMap->end())
        {
            // not found, so it's out of China Mainland
            qDebug() << host << "is not a China Mainland domain";
            m_proxyList.append(m_proxyCache);
            return m_proxyList;
        }
        domainMap = static_cast<DomainMap*>(domainMap->value(*it));
    }

    m_proxyList.append(QNetworkProxy::NoProxy);
    return m_proxyList;
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
         
        DomainMap* domainMap = &m_chinaDomains;
        QString domain(segs.at(1));
        auto domainSegs = domain.split('.');
        for (auto it = domainSegs.rbegin(); domainSegs.rend() != it; ++it)
        {
            if (domainMap->find(*it) == domainMap->end())
            {
                domainMap->insert(*it, new DomainMap);
            }
            domainMap = static_cast<DomainMap*>(domainMap->value(*it));
        }
    }
}
