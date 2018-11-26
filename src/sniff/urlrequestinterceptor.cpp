#include <QtCore>
#include <QUrl>
#include <QTextStream>
#include "urlrequestinterceptor.h"

UrlRequestInterceptor::UrlRequestInterceptor(QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent)
{
}

void UrlRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &request)
{
    //qDebug() << "1st" <<request.resourceType() << request.requestUrl();
    if (!request.requestUrl().url().startsWith("http"))
        return;

    switch (request.resourceType()) {
    case QWebEngineUrlRequestInfo::ResourceTypeMainFrame:
    case QWebEngineUrlRequestInfo::ResourceTypeSubFrame:
    case QWebEngineUrlRequestInfo::ResourceTypeStylesheet:
    case QWebEngineUrlRequestInfo::ResourceTypeScript:
    case QWebEngineUrlRequestInfo::ResourceTypeImage:
    case QWebEngineUrlRequestInfo::ResourceTypeFontResource:
    case QWebEngineUrlRequestInfo::ResourceTypePrefetch:
    case QWebEngineUrlRequestInfo::ResourceTypeFavicon:
    case QWebEngineUrlRequestInfo::ResourceTypeXhr:
    case QWebEngineUrlRequestInfo::ResourceTypePing:
        return;
    case QWebEngineUrlRequestInfo::ResourceTypePluginResource:
    case QWebEngineUrlRequestInfo::ResourceTypeMedia:
        qDebug() << "maybe:" << request.resourceType() << request.requestUrl();
        break;
    default:
        break;
    }

    QUrl url = request.requestUrl();
    QString u = url.url();
    QString path = url.path().toLower();

    if (path.endsWith("m3u8"))
    {
        emit maybeMediaUrl(request.requestUrl().url());
        QTextStream ts(stdout);
        ts << request.requestUrl().url();
        qApp->quit();
        return;
    }
    if ((url.hasQuery() || u.length() > 60) && (path.endsWith("mp4") || path.endsWith("flv")))
    {
        emit maybeMediaUrl(request.requestUrl().url());
        QTextStream ts(stdout);
        ts << request.requestUrl().url();
        qApp->quit();
        return;
    }

//    Websites& shortcuts = Browser::instance().shortcuts();
//    if (shortcuts.isInChina(url))
//    {
//        if (path.endsWith(".f4v") || path.endsWith(".mp4") || path.endsWith(".swf") || path.endsWith(".flv"))
//        {
//            request.block(true);
//            return;
//        }
//    }

    qDebug() << "maybe not:" << request.resourceType() << request.requestUrl();
}
