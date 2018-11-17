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
        emit inputEnd();
    });
    connect(&m_ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this](){
        emit inputEnd();
    });
    connect(&m_ffmpegProcess, &QProcess::readyReadStandardOutput, this, &MediaRelay::onReadStandardOutput);
}

QString MediaRelay::makeM3U8(PlayerPtr player, QStringList &urls)
{
    InMemoryHandler& httpHandler = Browser::instance().m_httpHandler;

    int duration = 1500 / urls.length();
    QByteArray m3u8;
    m3u8.append(QString("#EXTM3U\n#EXT-X-TARGETDURATION:%1\n").arg(duration > 8 ? duration + 3 : 8).toUtf8());
    if (player->type() == Player::PT_DLNA)
    {
        for (const auto & u : urls)
        {
            m3u8.append(QString("#EXTINF:%1,\n%2\n")
                        .arg(duration > 5 ? duration : 5)
                        .arg(httpHandler.mapUrl(u))
                        .toUtf8());
        }
    }
    else
    {
        for (const auto & u : urls)
        {
            m3u8.append(QString("#EXTINF:%1,\n%2\n")
                        .arg(duration > 5 ? duration : 5)
                        .arg(u)
                        .toUtf8());
        }
    }
    m3u8.append("#EXT-X-ENDLIST\n");
    httpHandler.setM3U8(m3u8);
    return QString("http://%1:51290/media.m3u8").arg(Util::getLocalAddress().toString());
}

QString MediaRelay::transcoding(const QString& media)
{
    m_ffmpegProcess.kill();
    m_ffmpegProcess.setProgram(Config().read<QString>("ffmpeg"));
    m_ffmpegProcess.setArguments(QStringList() << "-y"
                                 << "-protocol_whitelist" << "file,http,https,tcp,tls"
                                 << "-i" << media
                                 << "-c" << "copy"
                                 << "-copyts"
                                 << "-f" << "mpegts"
                                 << "-"
                                 //<< QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/media.ts")
                                 );
    m_ffmpegProcess.setProcessChannelMode(QProcess::SeparateChannels);
    m_ffmpegProcess.start();
    // serve http://...:51290/media.ts
    return QString("http://%1:51290/media.ts").arg(Util::getLocalAddress().toString());
}

void MediaRelay::processM3U8(const QString &media, const QByteArray& userAgent, const QByteArray& referrer)
{
    qDebug() << __FUNCTION__ << media;
    m_socketData.clear();
    QNetworkRequest req;
    QUrl u(media);
    req.setUrl(u);
    req.setRawHeader("User-Agent", userAgent);
    req.setRawHeader("Referer", referrer);
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QNetworkAccessManager &nam = Browser::instance().networkAccessManager();
    QNetworkReply *reply = nam.get(req);
    connect(reply, &QIODevice::readyRead, this, &MediaRelay::onReadyRead);
    connect(reply, &QNetworkReply::finished, this, &MediaRelay::onMediaReadFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &MediaRelay::onNetworkError);
    connect(reply, &QNetworkReply::sslErrors, this, &MediaRelay::onNetworkSSLErrors);
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
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    m_socketData.append(reply->readAll());
}

void MediaRelay::onMediaReadFinished()
{
    InMemoryHandler& httpHandler = Browser::instance().m_httpHandler;
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    m_socketData.append(reply->readAll());

    reply->deleteLater();
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
                int index = originUrl.lastIndexOf('/');
                u = originUrl.left(index + 1) + u;
            }
            // u is an absolute path
            if (QUrl(u).path().endsWith("m3u8", Qt::CaseInsensitive))
            {
                QByteArray userAgent = reply->request().rawHeader("User-Agent");
                QByteArray referrer = reply->request().rawHeader("Referer");
                processM3U8(u, userAgent, referrer);
                // suppose there is only one m3u8 link
                return;
            }
            else
            {
                u = httpHandler.mapUrl(u);
                m3u8 = m3u8.replace(line, u.toUtf8());
            }
        }
    }
    httpHandler.setM3U8(m3u8);

    emit newM3U8Ready();
}

void MediaRelay::onReadStandardOutput()
{
    InMemoryHandler& httpHandler = Browser::instance().m_httpHandler;
    httpHandler.newMediaData(m_ffmpegProcess.readAllStandardOutput());
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
