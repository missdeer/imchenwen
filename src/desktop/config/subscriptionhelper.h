#ifndef SUBSCRIPTIONHELPER_H
#define SUBSCRIPTIONHELPER_H

#include <QObject>
#include <QSharedPointer>
#include <QStringList>
#include <QMultiMap>
#include <QNetworkReply>
#include "config.h"

typedef Tuple2List SubscriptionList;
typedef QSharedPointer<SubscriptionList> SubscriptionListPtr;
typedef QMultiMap<QString, SubscriptionListPtr> SubscriptionContentMap;

class SubscriptionHelper : public QObject
{
    Q_OBJECT
public:
    SubscriptionHelper(const QString& customKey, const QString& subscriptionKey, QObject *parent = nullptr);
    void setCustomKey(const QString& key);
    void setSubscriptionKey(const QString& key);
    void update();
    SubscriptionContentMap& content() { return  m_content; }

    bool parseAsJSON(const QByteArray &data);
    bool parseAsPlainText(const QByteArray &data, const QString& title);
signals:
    void ready();
private slots:
    void onNetworkError(QNetworkReply::NetworkError code);
    void onNetworkSSLErrors(const QList<QSslError> &errors);
    void onReadyRead();
    void onReadFinished();

private:
    QString m_customKey;
    QString m_subscriptionKey;
    SubscriptionContentMap m_content;
    QByteArray m_data;
    void requestSubscription(QStringList *subscriptItems, int index);
};

#endif // SUBSCRIPTIONHELPER_H
