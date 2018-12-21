#include "websites.h"
#include "browser.h"
#include "config.h"
#include <QtCore>
#include <QDomDocument>
#include <QDomNode>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

Websites::Websites(QObject *parent)
    : QObject(parent)
{
}

bool Websites::isIn(const QUrl &url, const QString& category)
{
    QString host = url.host();
    auto it = std::find_if(m_websites.begin(), m_websites.end(), [&host](WebsitePtr w) {
            QStringList ss = host.split(QChar('.'));
            while (ss.length() > 1)
            {
                if (w->url.contains(ss.join(QChar('.')), Qt::CaseInsensitive))
                    return true;
                ss.removeAt(0);
            }
            return false;
    });

    if (m_websites.end() != it)
        return category.isEmpty() ? true : (*it)->category == category;

    return false;
}

void Websites::parseWebsiteNode(QDomElement website, const QString& category)
{
    while(!website.isNull())
    {
        WebsitePtr w(new Website);
        QDomElement nameElem = website.firstChildElement("name");
        w->title = nameElem.text();
        QDomElement urlElem = website.firstChildElement("url");
        w->url = urlElem.text();
        QDomElement favNode = website.firstChildElement("favourite");
        w->favourite = !favNode.isNull();
        w->category = category;
        m_websites.push_back(w);
        website = website.nextSiblingElement("website");
    }
}

void Websites::doParse()
{
    QDomDocument doc("websites");
    if (!doc.setContent(m_data))
    {
        qCritical() << "can't set websites configuration content to dom";
        return ;
    }

    // print out the element names of all elements that are direct children
    // of the outermost element.
    QDomElement docElem = doc.documentElement();

    QStringList cats = {"china", "abroad", "film"};
    for (const auto & cat : cats)
    {
        QDomElement node = docElem.firstChildElement(cat);
        if (node.isNull())
        {
            qCritical() << "no " << cat << " node";
            continue ;
        }

        QDomElement website = node.firstChildElement("website");
        parseWebsiteNode(website, cat);
    }

    if (!m_websites.isEmpty())
        emit ready();
}

void Websites::update()
{
    m_data.clear();
    m_websites.clear();
    QNetworkRequest req;
    Config cfg;
    QUrl u(cfg.read<QString>(QLatin1String("shortcut")));
    req.setUrl(u);
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QNetworkAccessManager &nam = Browser::instance().networkAccessManager();
    QNetworkReply *reply = nam.get(req);
    connect(reply, &QIODevice::readyRead, this, &Websites::onReadyRead);
    connect(reply, &QNetworkReply::finished, this, &Websites::onReadFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &Websites::onNetworkError);
    connect(reply, &QNetworkReply::sslErrors, this, &Websites::onNetworkSSLErrors);
}

const QString &Websites::findURL(const QString &name)
{
    auto it = std::find_if(m_websites.begin(), m_websites.end(), [&name](WebsitePtr w) { return w->title == name;});
    return (*it)->url;
}

WebsiteList Websites::favourites()
{
    WebsiteList res;
    std::copy_if(m_websites.begin(), m_websites.end(), std::back_inserter(res), [](WebsitePtr w) { return w->favourite;});
    return res;
}

WebsiteList Websites::inChina()
{
    WebsiteList res;
    std::copy_if(m_websites.begin(), m_websites.end(), std::back_inserter(res), [](WebsitePtr w) { return w->category == "china";});
    return res;
}

WebsiteList Websites::abroad()
{
    WebsiteList res;
    std::copy_if(m_websites.begin(), m_websites.end(), std::back_inserter(res), [](WebsitePtr w) { return w->category == "abroad";});
    return res;
}

WebsiteList Websites::onlineFilm()
{
    WebsiteList res;
    std::copy_if(m_websites.begin(), m_websites.end(), std::back_inserter(res), [](WebsitePtr w) { return w->category == "film";});
    return res;
}

void Websites::onNetworkError(QNetworkReply::NetworkError code)
{
    qWarning() << code;
}

void Websites::onNetworkSSLErrors(const QList<QSslError> &errors)
{
    for (auto & e : errors)
        qWarning() << e.errorString();
}

void Websites::onReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    m_data.append( reply->readAll());
}

void Websites::onReadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();
    onReadyRead();
    doParse();
}
