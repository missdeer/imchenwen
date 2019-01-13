#ifndef INGFWLISTPROXYFACTORY_H
#define INGFWLISTPROXYFACTORY_H

#include <QNetworkProxyFactory>
#include "proxyrule.h"

class InGFWListProxyFactory : public QObject, public QNetworkProxyFactory
{
    Q_OBJECT
public:
    InGFWListProxyFactory();
    ~InGFWListProxyFactory();
    void init();
    void updateProxyCache(const QNetworkProxy &proxy);
    QList<QNetworkProxy> queryProxy ( const QNetworkProxyQuery & query = QNetworkProxyQuery() ) ;
    bool needProxy(const QString& url);

private slots:
    void done();

private:
    QNetworkProxy m_proxyCache;
    QList<ProxyRule*> m_proxyRules;
    QList<ProxyRule*> m_modifiedRules;
    QList<ProxyRule*> m_excludingProxyRules;
    QList<ProxyRule*> m_includingProxyRules;
    bool isAutoProxyRuleMatched(const QUrl &url);
    void newRule( ProxyRule * rule );
};

#endif // INGFWLISTPROXYFACTORY_H
