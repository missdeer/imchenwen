#include <QtCore>
#include <QUrl>
#include <QTextStream>
#include "urlrequestinterceptor.h"

UrlRequestInterceptor::UrlRequestInterceptor(QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent)
{
}

void UrlRequestInterceptor::outputToStdout(const QString &output)
{
    QTextStream ts(stdout);
    ts << output;
}

void UrlRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &request)
{
    qDebug() << "1st" <<request.resourceType() << request.requestUrl();
    if (!request.requestUrl().url().startsWith("http"))
        return;

    QUrl url = request.requestUrl();
    QString u = url.url();
    QString path = url.path().toLower();

    if (path.endsWith("m3u8"))
    {
        emit maybeMediaUrl(request.requestUrl().url());
        outputToStdout(request.requestUrl().url());
        qApp->quit();
        return;
    }
    if ((url.hasQuery() || u.length() > 60) && (path.endsWith("mp4") || path.endsWith("flv")))
    {
        emit maybeMediaUrl(request.requestUrl().url());
        outputToStdout(request.requestUrl().url());
        qApp->quit();
        return;
    }

    qDebug() << "maybe not:" << request.resourceType() << request.requestUrl();
}
