#include "websites.h"
#include <QtCore>
#include <QDomDocument>
#include <QDomNode>

bool Websites::isInChina(const QUrl &url)
{
    QString host = url.host();
    int pos = host.lastIndexOf(QChar('.'));
    pos = host.lastIndexOf(QChar('.'), pos-host.length()-1);
    host = host.mid(pos+1, -1);
    auto it = std::find_if(m_websites.begin(), m_websites.end(),
                           [&host](WebsitePtr w) { return w->url.contains(host); });

    if (m_websites.end() != it)
        return (*it)->inChina;

    pos = host.lastIndexOf(QChar('.'));
    host = host.left(pos);
    it = std::find_if(m_websites.begin(), m_websites.end(),
                           [&host](WebsitePtr w) { return w->url.contains(host); });

    if (m_websites.end() != it)
        return (*it)->inChina;

    return false;
}

const QString &Websites::findURL(const QString &name)
{
    auto it = std::find_if(m_websites.begin(), m_websites.end(), [&name](WebsitePtr w) { return w->name == name;});
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
    std::copy_if(m_websites.begin(), m_websites.end(), std::back_inserter(res), [](WebsitePtr w) { return w->inChina;});
    return res;
}

WebsiteList Websites::abroad()
{
    WebsiteList res;
    std::copy_if(m_websites.begin(), m_websites.end(), std::back_inserter(res), [](WebsitePtr w) { return !w->inChina;});
    return res;
}

Websites::Websites()
{
    doParse();
}

bool Websites::isIn(const QUrl &url)
{
    QString host = url.host();
    int pos = host.lastIndexOf(QChar('.'));
    pos = host.lastIndexOf(QChar('.'), pos-host.length()-1);
    host = host.mid(pos+1, -1);
    auto it = std::find_if(m_websites.begin(), m_websites.end(),
                           [&host](WebsitePtr w) { return w->url.contains(host); });

    if (m_websites.end() != it)
        return true;

    pos = host.lastIndexOf(QChar('.'));
    host = host.left(pos);
    it = std::find_if(m_websites.begin(), m_websites.end(),
                           [&host](WebsitePtr w) { return w->url.contains(host); });

    if (m_websites.end() != it)
        return true;

    return false;
}

void Websites::parseWebsiteNode(QDomElement website, bool inChina)
{
    while(!website.isNull())
    {
        WebsitePtr w(new Website);
        QDomElement nameElem = website.firstChildElement("name");
        w->name = nameElem.text();
        QDomElement urlElem = website.firstChildElement("url");
        w->url = urlElem.text();
        QDomElement favNode = website.firstChildElement("favourite");
        w->favourite = !favNode.isNull();
        w->inChina = inChina;
        m_websites.push_back(w);
        website = website.nextSiblingElement("website");
    }
}

void Websites::doParse()
{
    QDomDocument doc("websites");
    QFile file(":websites.xml");
    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical() << "can't open websites configuration file";
        return ;
    }

    if (!doc.setContent(&file))
    {
        qCritical() << "can't set websites configuration content to dom";
        file.close();
        return ;
    }
    file.close();

    // print out the element names of all elements that are direct children
    // of the outermost element.
    QDomElement docElem = doc.documentElement();

    QDomElement chinaNode = docElem.firstChildElement("china");
    if (chinaNode.isNull())
    {
        qCritical() << "no china node";
        return ;
    }

    QDomElement website = chinaNode.firstChildElement("website");
    parseWebsiteNode(website, true);

    QDomElement abroadNode = docElem.firstChildElement("abroad");
    if (abroadNode.isNull())
    {
        qCritical() << "no abroad node";
        return ;
    }
    website = abroadNode.firstChildElement("website");
    parseWebsiteNode(website, false);
}
