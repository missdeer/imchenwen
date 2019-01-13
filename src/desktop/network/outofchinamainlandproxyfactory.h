#ifndef OUTOFCHINAMAINLANDPROXYFACTORY_H
#define OUTOFCHINAMAINLANDPROXYFACTORY_H

#include <QNetworkProxyFactory>
#include <QNetworkReply>
#include <QObject>

typedef QMap<QString, void*> DomainMap; // void* => DomainMap*

class OutOfChinaMainlandProxyFactory : public QObject, public QNetworkProxyFactory
{
    Q_OBJECT
public:
    OutOfChinaMainlandProxyFactory();
    void init();
    void updateProxyCache(const QNetworkProxy &proxy);
    QList<QNetworkProxy> queryProxy ( const QNetworkProxyQuery & query = QNetworkProxyQuery() ) ;
    bool needProxy(const QString& url);
private slots:
    void done();

private:
     DomainMap m_chinaDomains;
     QNetworkProxy m_proxyCache;
};

#endif // OUTOFCHINAMAINLANDPROXYFACTORY_H
