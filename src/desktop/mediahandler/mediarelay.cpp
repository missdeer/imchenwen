#include "mediarelay.h"
#include "browser.h"
#include "inmemoryhandler.h"
#include "util.h"
#include <QDir>
#include <QStandardPaths>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

MediaRelay::MediaRelay(QObject *parent) : QObject(parent)
{
    connect(&m_ffmpegProcess, &QProcess::errorOccurred, [this](){
        emit transcodingFailed();
    });
    connect(&m_ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this](int exitCode, QProcess::ExitStatus status){
        if (exitCode != 0 || status != QProcess::NormalExit )
            emit transcodingFailed();
        else
            emit inputEnd();
    });
    connect(&m_ffmpegProcess, &QProcess::readyReadStandardOutput, this, &MediaRelay::onReadStandardOutput);
}

QString MediaRelay::makeOnlineM3U8(const QStringList &urls)
{
    InMemoryHandler& httpHandler = Browser::instance().m_httpHandler;

    int duration = 1500 / urls.length();
    QByteArray m3u8;
    m3u8.append(QString("#EXTM3U\n#EXT-X-TARGETDURATION:%1\n").arg(duration > 8 ? duration + 3 : 8).toUtf8());
    for (const auto & u : urls)
    {
        m3u8.append(QString("#EXTINF:%1,\n%2\n")
                    .arg(duration > 5 ? duration : 5)
                    .arg(httpHandler.mapUrl(u))
                    .toUtf8());
    }
    m3u8.append("#EXT-X-ENDLIST\n");
    httpHandler.setM3U8(m3u8);
    return QString("http://%1:51290/media.m3u8").arg(Util::getLocalAddress().toString());
}

QString MediaRelay::makeLocalM3U8(const QStringList &urls)
{
    int duration = 1500 / urls.length();
    QByteArray m3u8;
    m3u8.append(QString("#EXTM3U\n#EXT-X-TARGETDURATION:%1\n").arg(duration > 8 ? duration + 3 : 8).toUtf8());
    for (const auto & u : urls)
    {
        m3u8.append(QString("#EXTINF:%1,\n%2\n")
                    .arg(duration > 5 ? duration : 5)
                    .arg(u)
                    .toUtf8());
    }
    m3u8.append("#EXT-X-ENDLIST\n");

    QString path = QStandardPaths::writableLocation(QStandardPaths::TempLocation) % "/" % QUuid::createUuid().toString(QUuid::WithoutBraces) % ".m3u8";
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        f.write(m3u8);
        f.close();
    }
    return QDir::toNativeSeparators(path);
}

QString MediaRelay::transcoding(const QString& media)
{
    m_ffmpegProcess.kill();
    m_ffmpegProcess.setProgram(Config().read<QString>("ffmpeg"));
    QStringList args;
    Config cfg;
    if (cfg.read<bool>(QLatin1String("enableFFmpegHWAccel"), false))
    {
        args << "-hwaccel" << cfg.read<QString>(QLatin1String("ffmpegHWAccel"));
    }
    args << "-y"
         << "-protocol_whitelist" << "file,http,https,tcp,tls,crypto"
         << "-i" << media
         << "-c" << "copy"
         << "-copyts"
         << "-f" << "mpegts"
         << "-";
    m_ffmpegProcess.setArguments(args);
    m_ffmpegProcess.setProcessChannelMode(QProcess::SeparateChannels);
    m_ffmpegProcess.start();
    qDebug() << __FUNCTION__ << m_ffmpegProcess.arguments();
    // serve http://...:51290/media.ts
    return QString("http://%1:51290/media.ts").arg(Util::getLocalAddress().toString());
}

QString MediaRelay::merge(const QString &videoUrl, const QString &audioUrl, const QString &subtitleUrl)
{
    m_ffmpegProcess.kill();
    m_ffmpegProcess.setProgram(Config().read<QString>("ffmpeg"));
    QStringList args;
    Config cfg;
    if (cfg.read<bool>(QLatin1String("enableFFmpegHWAccel"), false))
    {
        args << "-hwaccel" << cfg.read<QString>(QLatin1String("ffmpegHWAccel"));
    }
    args << "-y"
         << "-protocol_whitelist" << "file,http,https,tcp,tls,crypto"
         << "-i" << videoUrl
         << "-i" << audioUrl;
    if (QUrl(subtitleUrl).isValid())
        args << "-i" << subtitleUrl
             << "-c:s mov_text";
    args << "-c" << "copy"
         << "-copyts"
         << "-f" << "mpegts"
         << "-";
    m_ffmpegProcess.setArguments(args);
    m_ffmpegProcess.setProcessChannelMode(QProcess::SeparateChannels);
    m_ffmpegProcess.start();
    qDebug() << __FUNCTION__ << m_ffmpegProcess.arguments();
    // serve http://...:51290/media.ts
    return QString("http://%1:51290/media.ts").arg(Util::getLocalAddress().toString());
}

void MediaRelay::processM3U8(const QString &media, const QByteArray& userAgent, const QByteArray& referrer)
{
    qDebug() << __FUNCTION__ << media << userAgent << referrer;
    m_socketData.clear();
    QNetworkRequest req;
    QUrl u(media);
    req.setUrl(u);
    req.setRawHeader("User-Agent", userAgent);
    req.setRawHeader("Referer", referrer);
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::UserVerifiedRedirectPolicy);
    req.setAttribute(QNetworkRequest::HTTP2AllowedAttribute, true);

    QNetworkAccessManager &nam = Browser::instance().networkAccessManager();
    QNetworkReply *reply = nam.get(req);
    connect(reply, &QIODevice::readyRead, this, &MediaRelay::onReadyRead);
    connect(reply, &QNetworkReply::finished, this, &MediaRelay::onMediaReadFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &MediaRelay::onNetworkError);
    connect(reply, &QNetworkReply::sslErrors, this, &MediaRelay::onNetworkSSLErrors);
    connect(reply, &QNetworkReply::redirected, this, &MediaRelay::onRedirected);
}

void MediaRelay::stop()
{
    m_ffmpegProcess.kill();
}

void MediaRelay::onNetworkError(QNetworkReply::NetworkError code)
{
    qDebug() << __FUNCTION__ << code;
}

void MediaRelay::onNetworkSSLErrors(const QList<QSslError> &errors)
{
    for (auto & e : errors)
        qWarning() << e.errorString();
}

void MediaRelay::onReadyRead()
{
    auto reply = qobject_cast<QNetworkReply*>(sender());
    m_socketData.append(reply->readAll());
}

void MediaRelay::onMediaReadFinished()
{
    InMemoryHandler& httpHandler = Browser::instance().m_httpHandler;
    auto reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();
    onReadyRead();

    qDebug() << __FUNCTION__ << QString(m_socketData);
    // parse the m3u8
    QByteArray m3u8 = m_socketData;
    auto lines = m_socketData.split('\n');
    for (auto & line : lines)
    {
        if (!line.isEmpty() && !line.startsWith('#'))
        {
            QString u = QString(line);
            if (!line.startsWith("http://") && !line.startsWith("https://"))
            {
                // relative path
                QString originUrl = reply->request().url().url();
                if (line.startsWith('/'))
                {
                    u = originUrl.replace(reply->request().url().path(), u);
                }
                else
                {
                    int index = originUrl.lastIndexOf('/');
                    u = originUrl.left(index + 1) + u;
                }
            }
            if (u.endsWith('?'))
                u = u.left(u.length() - 1);
            // u is an absolute path
            if (QUrl(u).path().endsWith("m3u8", Qt::CaseInsensitive))
            {
                QByteArray userAgent = reply->request().rawHeader("User-Agent");
                QByteArray referrer = reply->request().rawHeader("Referer");
                processM3U8(u, userAgent, referrer);
                // suppose there is only one m3u8 link
                return;
            }

            u = httpHandler.mapUrl(u);
            m3u8 = m3u8.replace(line, u.toUtf8());
        }
    }
    httpHandler.setM3U8(m3u8);
    qDebug() << __FUNCTION__ << "set m3u8:" << QString(m3u8);
    emit newM3U8Ready();
}

void MediaRelay::onReadStandardOutput()
{
    InMemoryHandler& httpHandler = Browser::instance().m_httpHandler;
    httpHandler.newMediaData(m_ffmpegProcess.readAllStandardOutput());
}

void MediaRelay::onRedirected(const QUrl &url)
{
    Q_UNUSED(url);
    auto reply = qobject_cast<QNetworkReply*>(sender());
    emit reply->redirectAllowed();
}

const QString &MediaRelay::title() const
{
    return m_title;
}

void MediaRelay::setTitle(const QString &title)
{
    m_title = title;
}

PlayerPtr MediaRelay::player() const
{
    return m_player;
}

void MediaRelay::setPlayer(const PlayerPtr &player)
{
    m_player = player;
}
