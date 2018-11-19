#ifndef WEBSITES_H
#define WEBSITES_H

#include <QList>
#include <QSharedPointer>
#include <QDomElement>

struct Website
{
    QString name;
    QString url;
    bool favourite;
    bool inChina;
};

typedef QSharedPointer<Website> WebsitePtr;

typedef QList<WebsitePtr> WebsiteList;

class Websites
{
public:
    Websites();

    bool isIn(const QUrl &url);
    bool isInChina(const QUrl &url);
    const QString& findURL(const QString& name);
    WebsiteList favourites();
    WebsiteList inChina();
    WebsiteList abroad();
private:
    void doParse();
private:
    WebsiteList m_websites;
    void parseWebsiteNode(QDomElement website, bool inChina);
};

#endif // WEBSITES_H
