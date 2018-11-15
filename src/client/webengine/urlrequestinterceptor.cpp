#include <QtCore>
#include <QUrl>
#include "urlrequestinterceptor.h"

UrlRequestInterceptor::UrlRequestInterceptor(QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent)
{
    Config cfg;
    cfg.read("vipVideo", m_vipVideos);
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
        return;
    }
    if ((url.hasQuery() || u.length() > 60) && (path.endsWith("mp4") || path.endsWith("flv")))
    {
        emit maybeMediaUrl(request.requestUrl().url());
        return;
    }

    auto it = std::find_if(m_vipVideos.begin(), m_vipVideos.end(), [&u](const Tuple2& vv){
        return u.startsWith(std::get<1>(vv));
    });
    if(m_vipVideos.end() == it)
    {
        // it's not a vip video
        if (path.endsWith(".f4v") || path.endsWith(".mp4") || path.endsWith(".swf") || path.endsWith(".flv"))
        {
            request.block(true);
            return;
        }
    }
    qDebug() << "maybe not:" << request.resourceType() << request.requestUrl();
}

void UrlRequestInterceptor::updateVIPVideos()
{
    m_vipVideos.clear();
    Config cfg;
    cfg.read("vipVideo", m_vipVideos);
}
