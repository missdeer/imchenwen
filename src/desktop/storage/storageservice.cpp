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
    QString baseUrl = cfg.read<QString>("storageServiceAddress");
    if (!QUrl(baseUrl).isValid())
        return;
    for (const auto& videoUrl : video->urls)
        doSubmit(baseUrl, videoUrl, title + "." + video->container, referrer);
    if (audio && !audio->urls.isEmpty())
        doSubmit(baseUrl, audio->urls[0], title + "." + audio->container, referrer);
    doSubmit(baseUrl, subtitle, title+".vtt", referrer);
}

void StorageService::submit(const QString &videoUrl, const QString &title)
{
    Config cfg;
    if (!cfg.read<bool>("enableStorageService", false))
        return;
    QString baseUrl = cfg.read<QString>("storageServiceAddress");
    if (!QUrl(baseUrl).isValid())
        return;
    doSubmit(baseUrl, videoUrl, title+".mp4", videoUrl);
}

void StorageService::onSubmitted()
{
    NetworkReplyHelper* reply = qobject_cast<NetworkReplyHelper*>(sender());
    reply->deleteLater();
}

void StorageService::doSubmit(const QString &baseUrl, const QString &targetLink, const QString &saveAs, const QString &referrer)
{
    if(targetLink.isEmpty())
        return;

    QNetworkRequest req;
    QUrl u(baseUrl);
    QUrlQuery query;
    query.addQueryItem("jsonrpc", "2");
    query.addQueryItem("id", QUuid::createUuid().toString());
    query.addQueryItem("method", "system.multicall");
    QString params = "[[{\"methodName\":\"aria2.addUri\",\"params\":[[\""
            % targetLink
            % "\"],{\"out\":\""
            % saveAs
            % "\", \"referer\": \""
            % referrer
            % "\", \"user-agent\":\""
            % Config().read<QString>("httpUserAgent")
            % "\", \"auto-file-renaming\": true }]}]]";
    query.addQueryItem("params", QString(params.toUtf8().toBase64()));
    u.setQuery(query);
    req.setUrl(u);
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    auto reply = Browser::instance().networkAccessManager().get(req);
    auto helper = new NetworkReplyHelper(reply);
    connect(helper, &NetworkReplyHelper::done, this, &StorageService::onSubmitted);
}
