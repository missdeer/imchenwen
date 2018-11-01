#include <QtCore>
#include <QUrl>
#include "urlrequestinterceptor.h"

UrlRequestInterceptor::UrlRequestInterceptor(QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent)
{

}

void UrlRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &request)
{
    //qDebug() << "1st" <<request.resourceType() << request.requestUrl();
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
    QUrl u = request.requestUrl();
    QString path = u.path().toLower();
    if (path.endsWith("m3u8"))
    {
        emit maybeMediaUrl(request.requestUrl().url());
        return;
    }
    if ((u.hasQuery() || u.url().length() > 60) && (path.endsWith("mp4") || path.endsWith("flv")))
    {
        emit maybeMediaUrl(request.requestUrl().url());
        return;
    }
    if (path.endsWith(".f4v"))
    {
        request.block(true);
    }
    qDebug() << "maybe not:" << request.resourceType() << request.requestUrl();
}
