#include <QtCore>
#include <QUrl>
#include <QTextStream>
#include <QProcess>
#include "urlrequestinterceptor.h"

UrlRequestInterceptor::UrlRequestInterceptor(bool mpv, QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent)
    , m_mpv(mpv)
{
}

void UrlRequestInterceptor::outputToStdout(const QString &output)
{
    QTextStream ts(stdout);
    ts << output;
}

void UrlRequestInterceptor::playByMPV(const QString &output)
{
#if defined(Q_OS_WIN)
    QProcess::startDetached(
        "mpv.exe",
        QStringList()
            << "--hwdec=dxva2-copy"
            << "--no-ytdl"
            << "--title=media from sniff"
            << "--force-media-title=media from sniff"
            << "--user-agent=Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.3994.0 Safari/537.36"
            << "--prefetch-playlist=yes" << output);
#elif defined(Q_OS_MAC)
    QProcess::startDetached(
        "mpv",
        QStringList()
            << "--hwdec=videotoolbox-copy"
            << "--no-ytdl"
            << "--title=media from sniff"
            << "--force-media-title=media from sniff"
            << "--user-agent=Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.3994.0 Safari/537.36"
            << "--prefetch-playlist=yes" << output);
#else
    QProcess::startDetached(
        "mpv",
        QStringList()
            << "--no-ytdl"
            << "--title=media from sniff"
            << "--force-media-title=media from sniff"
            << "--user-agent=Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.3994.0 Safari/537.36"
            << "--prefetch-playlist=yes" << output);
#endif
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
        if (m_mpv)
            playByMPV(request.requestUrl().url());
        else
            outputToStdout(request.requestUrl().url());
        qApp->quit();
        return;
    }
    if ((url.hasQuery() || u.length() > 60) && (path.endsWith("mp4") || path.endsWith("flv")))
    {
        emit maybeMediaUrl(request.requestUrl().url());
        if (m_mpv)
            playByMPV(request.requestUrl().url());
        else
            outputToStdout(request.requestUrl().url());
        qApp->quit();
        return;
    }

    qDebug() << "maybe not:" << request.resourceType() << request.requestUrl();
}
