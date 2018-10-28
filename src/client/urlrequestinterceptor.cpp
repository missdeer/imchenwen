#include <QtCore>
#include <QUrl>
#include "urlrequestinterceptor.h"

UrlRequestInterceptor::UrlRequestInterceptor(QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent)
{

}

void UrlRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &request)
{
    QUrl u = request.requestUrl();
    QString path = u.path().toLower();
    if (path.endsWith("m3u8"))
    {
        emit maybeMediaUrl(request.requestUrl().url());
        return;
    }
    if (u.hasQuery() && (path.endsWith("mp4") || path.endsWith("flv")))
    {
        emit maybeMediaUrl(request.requestUrl().url());
        return;
    }
    qDebug() << request.requestUrl();
}
