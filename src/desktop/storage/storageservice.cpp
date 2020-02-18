#include "storageservice.h"
#include "config.h"
#include "networkreplyhelper.h"
#include "browser.h"
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QUuid>
#include <QStringBuilder>

StorageService::StorageService(QObject *parent) : QObject(parent)
{

}

void StorageService::submit(StreamInfoPtr video, StreamInfoPtr audio, const QString &subtitle, const QString &title, const QString &referrer)
{
    Config cfg;
    if (!cfg.read<bool>("enableStorageService", false))
        return;
    if (cfg.read<QString>("aria2Type") == "rpc")
    {
        QString baseUrl = cfg.read<QString>("aria2RPCAddress");
        if (!QUrl(baseUrl).isValid())
        {
            qDebug() << "invalid baseUrl:" << baseUrl;
            return;
        }
        QString name = baseName(title);
        if (video->urls.length() == 1)
        {
            if (QUrl(video->urls.at(0)).isValid())
                doSubmit(baseUrl, video->urls.at(0), name % "." % video->container, referrer);
        }
        else
        {
            for (int index = 0; index < video->urls.length(); ++index)
            {
                const QString& videoUrl = video->urls.at(index);
                if (QUrl(videoUrl).isValid())
                    doSubmit(baseUrl, videoUrl, QString("%1-%2.%3").arg(name).arg(index).arg(video->container), referrer);
            }
        }
        if (audio && !audio->urls.isEmpty() && QUrl(audio->urls[0]).isValid())
            doSubmit(baseUrl, audio->urls[0], name % "." % audio->container, referrer);
        if (QUrl(subtitle).isValid())
            doSubmit(baseUrl, subtitle, name % ".vtt", referrer);
    }
    else 
    {
        QString aria2c = cfg.read<QString>("aria2ExecutablePath");
        QString savePath = cfg.read<QString>("downloadSavePath");
        
        QString name = baseName(title);
        if (video->urls.length() == 1)
        {
            if (QUrl(video->urls.at(0)).isValid())
            {
                QStringList args = {
                    "-x", "16", "-s", "16", "--referer=" + referrer, "--user-agent=" + cfg.read<QString>("httpUserAgent"), "-d", savePath};
                if (cfg.read<bool>("renameStorageFile"))
                    args << "-o" << name % "." % video->container;
                args << video->urls.at(0);
                QProcess::startDetached(aria2c, args);
            }
        }
        else
        {
            for (int index = 0; index < video->urls.length(); ++index)
            {
                const QString& videoUrl = video->urls.at(index);
                if (QUrl(videoUrl).isValid())
                {
                    QStringList args = {
                        "-x", "16", "-s", "16", "--referer=" + referrer, "--user-agent=" + cfg.read<QString>("httpUserAgent"), "-d", savePath};
                    if (cfg.read<bool>("renameStorageFile"))
                        args << "-o" << QString("%1-%2.%3").arg(name).arg(index).arg(video->container);
                    args << videoUrl;
                    QProcess::startDetached(aria2c, args);
                }
            }
        }
        if (audio && !audio->urls.isEmpty() && QUrl(audio->urls[0]).isValid())
        {
            QProcess::startDetached(aria2c,
                                    QStringList() << "-x"
                                                  << "16"
                                                  << "-s"
                                                  << "16"
                                                  << "--referer=" + referrer << "--user-agent=" + cfg.read<QString>("httpUserAgent") << "-d"
                                                  << savePath << "-o" << name % "." % audio->container << audio->urls.at(0));
        }
        if (QUrl(subtitle).isValid())
        {
            QProcess::startDetached(aria2c,
                                    QStringList() << "-x"
                                                  << "16"
                                                  << "-s"
                                                  << "16"
                                                  << "--referer=" + referrer << "--user-agent=" + cfg.read<QString>("httpUserAgent") << "-d"
                                                  << savePath << "-o" << name % ".vtt" << subtitle);
        }
    }
}

void StorageService::submit(const QString &videoUrl, const QString &title)
{
    Config cfg;
    if (!cfg.read<bool>("enableStorageService", false))
    {
        qDebug() << "enableStorageService == false";
        return;
    }

    if (cfg.read<QString>("aria2Type") == "rpc")
    {
        QString baseUrl = cfg.read<QString>("aria2RPCAddress");
        if (!QUrl(baseUrl).isValid())
        {
            qDebug() << "invalid baseUrl:" << baseUrl;
            return;
        }
        if (!QUrl(videoUrl).isValid())
        {
            qDebug() << "invalid videoUrl:" << videoUrl;
            return;
        }
        doSubmit(baseUrl, videoUrl, baseName(title) % ".mp4", videoUrl);
    }
    else 
    {
        QString aria2c = cfg.read<QString>("aria2ExecutablePath");
        QString     savePath = cfg.read<QString>("downloadSavePath");
        QStringList args     = {"-x", "16", "-s", "16", "-d", savePath};
        if (cfg.read<bool>("renameStorageFile"))
            args << "-o" << baseName(title) % ".mp4";
        args << videoUrl;
        QProcess::startDetached(aria2c, args);
    }
}

void StorageService::onSubmitted()
{
    NetworkReplyHelper* reply = qobject_cast<NetworkReplyHelper*>(sender());
    reply->deleteLater();
}

QString StorageService::baseName(const QString &base)
{
    const QString validChars = " -_.+(){}[]!@#$%^&;~";
    QString bn = base;
    for (int index = bn.length() - 1; index >=0; --index)
    {
        QChar ch = bn.at(index);
        if (!ch.isLetterOrNumber() && !validChars.contains(ch, Qt::CaseInsensitive))
            bn.remove(index, 1);
    }
    return bn;
}

QUuid StorageService::doSubmit(const QString &baseUrl, const QString &targetLink, const QString &saveAs, const QString &referrer)
{
    QUuid           uuid = QUuid::createUuid();
    QNetworkRequest req;
    QUrl u(baseUrl);
    req.setUrl(u);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    Config  cfg;
    QString userAgent = cfg.read<QString>("httpUserAgent");
    QString body      = QString("[{\"jsonrpc\":\"2.0\",\"method\":\"aria2.addUri\",\"id\":\"%1\",\"params\":[[\"%2\"],{\"referer\":\"%3\","
                           "\"user-agent\":\"%4\"}]}]")
                       .arg(uuid.toString(QUuid::WithoutBraces), targetLink, referrer, userAgent);
    if (cfg.read<bool>("renameStorageFile"))
        body = QString("[{\"jsonrpc\":\"2.0\",\"method\":\"aria2.addUri\",\"id\":\"%1\",\"params\":[[\"%2\"],{\"out\":\"%3\",\"referer\":\"%4\","
                       "\"user-agent\":\"%5\"}]}]")
                   .arg(uuid.toString(QUuid::WithoutBraces), targetLink, saveAs, referrer, userAgent);

    auto reply  = Browser::instance().networkAccessManager().post(req, body.toUtf8());
    auto helper = new NetworkReplyHelper(reply);
    connect(helper, &NetworkReplyHelper::done, this, &StorageService::onSubmitted);

    return uuid;
}
