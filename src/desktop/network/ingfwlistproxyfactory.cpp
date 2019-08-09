#include "ingfwlistproxyfactory.h"
#include "config.h"
#include "browser.h"
#include "networkreplyhelper.h"

struct ProxyRuleMatched
{
private:
    const QString& m_rulePattern;
public:
    explicit ProxyRuleMatched(const QString& rulePattern)
        : m_rulePattern(rulePattern)
    {}

    bool operator()(const ProxyRule* rule)
    {
        return rule->pattern() == m_rulePattern;
    }
};

InGFWListProxyFactory::InGFWListProxyFactory()
{

}

InGFWListProxyFactory::~InGFWListProxyFactory()
{
    foreach(ProxyRule* rule, m_modifiedRules) {
        if (rule->deleted()) {
            delete rule;
        }
    }
    m_modifiedRules.clear();

    foreach(ProxyRule* rule, m_proxyRules) {
        delete rule;
    }
    m_proxyRules.clear();
}

void InGFWListProxyFactory::init()
{
    // download gfwlist first
    Config cfg;
    QNetworkRequest req;
    QUrl u(cfg.read(QLatin1String("gfwList"), QString("https://cdn.jsdelivr.net/gh/gfwlist/gfwlist/gfwlist.txt")));
    req.setUrl(u);
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    req.setAttribute(QNetworkRequest::HTTP2AllowedAttribute, true);

    QNetworkAccessManager &nam = Browser::instance().networkAccessManager();
    auto reply = nam.get(req);
    auto helper = new NetworkReplyHelper(reply);
    connect(helper, &NetworkReplyHelper::done, this, &InGFWListProxyFactory::done);
}

void InGFWListProxyFactory::updateProxyCache(const QNetworkProxy &proxy)
{
    m_proxyCache = proxy;
}

QList<QNetworkProxy> InGFWListProxyFactory::queryProxy(const QNetworkProxyQuery &query)
{
    QList<QNetworkProxy> m_proxyList;
    if (isAutoProxyRuleMatched(query.url()))
        m_proxyList.append(m_proxyCache);
    else
        m_proxyList.append(QNetworkProxy::NoProxy);
    return m_proxyList;
}

bool InGFWListProxyFactory::needProxy(const QString &url)
{
    return isAutoProxyRuleMatched(QUrl::fromUserInput(url));
}

void InGFWListProxyFactory::done()
{
    NetworkReplyHelper* helper = qobject_cast<NetworkReplyHelper*>(sender());
    helper->deleteLater();

    QByteArray content = QByteArray::fromBase64( helper->content());

    QList<QByteArray> rules = content.split('\n');

    // fill into m_proxyRules
    for(const auto& rulePattern : rules)
    {
        if(rulePattern.length() <= 1 || rulePattern.at(0) == '!' || rulePattern.at(0) == '[')
            continue;

        QString strRulePattern(rulePattern.data());
        // check to see if it exists already? use the existed one
        QList<ProxyRule*>::iterator findIt = std::find_if(m_proxyRules.begin(),
                                                          m_proxyRules.end(),
                                                          ProxyRuleMatched(strRulePattern));
        if (m_proxyRules.end() != findIt )
            continue;

        ProxyRule *rule = new ProxyRule(strRulePattern);
        newRule(rule);
    }
}

bool InGFWListProxyFactory::isAutoProxyRuleMatched(const QUrl &url)
{
    if (url.toString().isEmpty())
        return true;
    QHostAddress host(url.host());
    if (!host.isNull() 
        || host.isInSubnet(QHostAddress("127.0.0.0"), 8)
        || host.isInSubnet(QHostAddress("192.168.0.0"), 16)
        || host.isInSubnet(QHostAddress("169.254.0.0"), 16)
        || host.isInSubnet(QHostAddress("224.0.0.0"), 4)
        || host.isInSubnet(QHostAddress("240.0.0.0"), 4)
        || host.isInSubnet(QHostAddress("10.0.0.0"), 8)
        || host.isInSubnet(QHostAddress("172.16.0.0"), 12))
        return false;
    // first check the excluding rules, because their priorities are the highest
    for(ProxyRule* rule: m_excludingProxyRules) {
        if (rule->isMatched(url)) {
            rule->increaseHits();
            if (!m_modifiedRules.contains(rule)) {
                m_modifiedRules.append(rule);
            }
            return false;
        }
    }
    // then check other rules
    for(ProxyRule* rule: m_includingProxyRules) {
        if (rule->isMatched(url)) {
            rule->increaseHits();
            if (!m_modifiedRules.contains(rule)) {
                m_modifiedRules.append(rule);
            }
            return true;
        }
    }

    // donot enable proxy by default
    return false;
}

void InGFWListProxyFactory::newRule(ProxyRule *rule)
{
    m_proxyRules.append(rule);

    if (rule->pattern().startsWith("@@")) {
        m_excludingProxyRules.append(rule);
    } else {
        m_includingProxyRules.append(rule);
    }
}
