#include "subscriptionhelper.h"
#include "config.h"
#include "browser.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

Q_DECLARE_METATYPE(QStringList *);

SubscriptionHelper::SubscriptionHelper(QObject *parent)
    : QObject(parent)
{

}

SubscriptionHelper::SubscriptionHelper(const QString& customKey, const QString& subscriptionKey, QObject *parent )
    : QObject (parent)
    , m_customKey(customKey)
    , m_subscriptionKey(subscriptionKey)
{

}

void SubscriptionHelper::setCustomKey(const QString &key)
{
    m_customKey = key;
}

void SubscriptionHelper::setSubscriptionKey(const QString &key)
{
    m_subscriptionKey = key;
}

void SubscriptionHelper::requestSubscription(QStringList *subscriptItems, int index)
{
    QNetworkRequest req;
    req.setAttribute(QNetworkRequest::User, index);
    req.setAttribute(static_cast<QNetworkRequest::Attribute>(QNetworkRequest::User + 1), QVariant::fromValue(subscriptItems));
    QUrl u(subscriptItems->at(index));
    req.setUrl(u);
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QNetworkAccessManager &nam = Browser::instance().networkAccessManager();
    QNetworkReply *reply = nam.get(req);
    connect(reply, &QIODevice::readyRead, this, &SubscriptionHelper::onReadyRead);
    connect(reply, &QNetworkReply::finished, this, &SubscriptionHelper::onReadFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &SubscriptionHelper::onNetworkError);
    connect(reply, &QNetworkReply::sslErrors, this, &SubscriptionHelper::onNetworkSSLErrors);
}

void SubscriptionHelper::update()
{
    m_content.clear();
    Config cfg;
    // read custom items
    SubscriptionListPtr customList(new SubscriptionList);
    cfg.read(m_customKey, *customList.data());
    if (!customList->isEmpty())
        m_content.insert(tr("Custom"), customList);

    // read subscription items
    QStringList *subscriptItems = new QStringList;
    cfg.read(m_subscriptionKey, *subscriptItems);
    if (subscriptItems->isEmpty())
    {
        emit ready();
        delete subscriptItems;
        return;
    }

    // get subscription content
    requestSubscription(subscriptItems, 0);
}

bool SubscriptionHelper::parseAsJSON(const QByteArray& data)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError)
    {
        return false;
    }
    if (!doc.isObject())
    {
        return false;
    }
    auto rootObj = doc.object();

    QString title = tr("unknown");

    auto titleObj = rootObj["title"];
    if (titleObj.isString())
        title = titleObj.toString();
    auto channels = rootObj["channels"];
    if (!channels.isArray())
        return false;
    auto arr = channels.toArray();
    SubscriptionListPtr sl(new SubscriptionList);
    for (auto a : arr)
    {
        if (!a.isObject())
            continue;
        auto o = a.toObject();
        if (!o["name"].isString() || o["name"].toString().isEmpty() || !o["url"].isString() || o["url"].toString().isEmpty())
            continue;
        if (!QUrl(o["url"].toString()).isValid())
            continue;
        sl->append(std::make_tuple(o["name"].toString(), o["url"].toString()));
    }
    m_content.insert(title, sl);

    return true;
}

bool SubscriptionHelper::parseAsPlainText(const QByteArray &data, const QString &title)
{
    auto lines = data.split('\n');
    SubscriptionListPtr sl(new SubscriptionList);
    for (const auto& line : lines)
    {
        auto ele = line.trimmed().split(' ');
        if (ele.length() <2 )
        {
            ele = line.trimmed().split(',');
            if (ele.length() <2 )
            {
                QUrl url(line.trimmed());
                if (url.isValid())
                {
                    auto vv = std::make_tuple(url.host(), url.url());
                    sl->append(vv);
                }
                continue;
            }
        }

        if (!QUrl(QString(ele[1])).isValid())
            continue;

        auto vv = std::make_tuple(QString(ele[0].trimmed()), QString(ele[1].trimmed()));
        sl->append(vv);
    }
    if (sl->isEmpty())
        return false;

    m_content.insert(title.isEmpty()?tr("unknown"):title, sl);
    return true;
}

void SubscriptionHelper::onNetworkError(QNetworkReply::NetworkError code)
{
    qWarning() << code;
}

void SubscriptionHelper::onNetworkSSLErrors(const QList<QSslError> &errors)
{
    for (auto & e : errors)
        qWarning() << e.errorString();
}

void SubscriptionHelper::onReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    m_data.append( reply->readAll());
}

void SubscriptionHelper::onReadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    m_data.append( reply->readAll());
    // parse as JSON
    if (!parseAsJSON(m_data))
    {
        // parse as plain text
        QFileInfo fi(reply->url().path());
        parseAsPlainText(m_data, fi.fileName());
    }

    m_data.clear();

    int index = reply->request().attribute(QNetworkRequest::User).toInt() + 1;
    QStringList* urls = reply->request().attribute(static_cast<QNetworkRequest::Attribute>(QNetworkRequest::User + 1)).value<QStringList*>();

    if (index >= urls->length())
    {
        delete urls;
        emit ready();
    }
    else
        requestSubscription(urls, index);

    reply->deleteLater();
}
