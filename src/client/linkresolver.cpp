#include "linkresolver.h"
#include "websites.h"
#include "config.h"
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

static QNetworkAccessManager nam;

LinkResolver::LinkResolver(QObject *parent)
    : QObject(parent)
{
}

void LinkResolver::resolve(const QUrl &url, bool silent)
{
    bool inChina = Websites::instance().isInChina(url);
    Config cfg;
    bool inChinaLocalMode = cfg.read<bool>("inChinaLocalMode");
    bool abroadLocalMode = cfg.read<bool>("abroadLocalMode");
    bool localMode = (inChina ? inChinaLocalMode : abroadLocalMode);
    auto it = std::find_if(m_history.begin(), m_history.end(),
                           [this, url, localMode](HistoryItemPtr h) {return h->url.startsWith(url.toString()) && h->localMode == localMode;});
    if (m_history.end() != it)
    {
        if ((*it)->time.secsTo(QTime::currentTime()) > 60 * 60)
        {
            m_history.erase(it);
        }
        else
        {
            emit resolvingFinished((*it)->mi);
            return;
        }
    }
    m_content.clear();
    QNetworkRequest req;
    if (inChina)
        req.setUrl(QUrl(inChinaLocalMode ? "http://127.0.0.1:8765/v1/parse" : "https://pcn.xyying.me/v1/parse"));
    else
        req.setUrl(QUrl(abroadLocalMode ? "http://127.0.0.1:8765/v1/parse" : "https://pjp.xyying.me/v1/parse"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    req.setRawHeader("RequestUrl", url.toString().toUtf8());
    req.setRawHeader("Silent", silent ? "true" : "false");
    req.setRawHeader("LocalMode", (inChina ? (inChinaLocalMode ? "true" : "false") : (abroadLocalMode ? "true" : "false")));
    QByteArray data;
    data.append("apikey=");
    QString apikey = cfg.read<QString>("apiKey");
    if (apikey.isEmpty())
        apikey = "yb2Q1ozScRfJJ";
    data.append(apikey);
    data.append(inChina ? "&parser=ykdl,you-get" : "&parser=youtube-dl,you-get");
    data.append("&url=");
    data.append(url.toString().toUtf8().toPercentEncoding());
    QNetworkReply *reply = nam.post(req, data);

    connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(error(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(sslErrors(QList<QSslError>)));
    connect(reply, SIGNAL(finished()),
            this, SLOT(finished()));
}

void LinkResolver::error(QNetworkReply::NetworkError code)
{
    qDebug() << "resolving error:" << code;
}

void LinkResolver::finished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();

    MediaInfoPtr mi(new MediaInfo);

    QNetworkRequest req = reply->request();
    QByteArray silent = req.rawHeader("Silent");

    QJsonDocument doc = QJsonDocument::fromJson(m_content);
    if (!doc.isObject())
    {
        qDebug() << "content received is not a json object" << QString(m_content);
        if (QString(silent) == "true")
            emit resolvingSilentError();
        else
            emit resolvingError();
        return;
    }

    QJsonObject docObj = doc.object();
    QJsonValue res = docObj["Result"];
    if (!res.isString())
    {
        qDebug() << "unexpect result node";
        if (QString(silent) == "true")
            emit resolvingSilentError();
        else
            emit resolvingError();
        return;
    }

    if (res.toString()!= "OK")
    {
        qDebug() << "resolving failed";
        if (QString(silent) == "true")
            emit resolvingSilentError();
        else
            emit resolvingError();
        return;
    }

    if (docObj["YouGet"].isObject())
    {
        parseNode(docObj["YouGet"].toObject(), mi, mi->you_get);
    }

    if (docObj["YKDL"].isObject())
    {
        parseNode(docObj["YKDL"].toObject(), mi, mi->ykdl);
    }

    if (docObj["YoutubeDL"].isObject())
    {
        parseNode(docObj["YoutubeDL"].toObject(), mi, mi->youtube_dl);
    }

    if (mi->title.isEmpty() && mi->site.isEmpty())
    {
        if (QString(silent) == "true")
            emit resolvingSilentError();
        else
            emit resolvingError();
        return;
    }

    QByteArray requestUrl = req.rawHeader("RequestUrl");
    bool localMode = false;
    if (QString(req.rawHeader("Silent")) == "true")
        localMode = true;
    m_history.push_back(HistoryItemPtr(new HistoryItem { QString(requestUrl), QTime::currentTime(), localMode, mi }));
    // already ordered by time
    // remove the old elements > 1 hour
    while (m_history[0]->time.secsTo(QTime::currentTime()) > 20 * 60)
        m_history.erase(m_history.begin());

    if (QString(silent) == "true")
        emit resolvingSilentFinished(mi);
    else
        emit resolvingFinished(mi);
}

void LinkResolver::sslErrors(const QList<QSslError> &errors)
{
    for (auto e : errors)
        qDebug() << "resovling ssl errors:" << e.errorString();
}

void LinkResolver::readyRead()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 200 && statusCode < 300) {
        m_content.append( reply->readAll());
    }
}

void LinkResolver::parseNode(const QJsonObject &o, MediaInfoPtr mi, Streams &streams)
{
    if (mi->site.isEmpty())
    {
        auto site = o["Site"];
        if (site.isString())
        {
            mi->site = site.toString();
        }
    }

    if (mi->title.isEmpty())
    {
        auto title = o["Title"];
        if (title.isString())
        {
            mi->title = title.toString();
        }
    }


    auto streamsArray = o["Streams"];
    if (!streamsArray.isArray())
    {
        qDebug() << "streams is expected to be an array";
        return;
    }

    auto ss = streamsArray.toArray();
    for (auto stream : ss)
    {
        if (!stream.isObject())
        {
            qDebug() << "stream is expected to be an object";
            continue;
        }
        auto s = stream.toObject();

        StreamInfoPtr si(new StreamInfo);
        auto containerIt = s.find("Container");
        if (s.end() != containerIt)
        {
            if (containerIt->isString())
            {
                si->container = containerIt->toString();
            }
        }

        auto qualityIt = s.find("Quality");
        if (s.end() != qualityIt)
        {
            if (qualityIt->isString())
            {
                si->quality = qualityIt->toString();
            }
        }

        auto videoProfileIt = s.find("VideoProfile");
        if (s.end() != videoProfileIt)
        {
            if (videoProfileIt->isString())
            {
                si->quality = videoProfileIt->toString();
            }
        }

        auto urlsIt = s.find("RealURLs");
        if (s.end() == urlsIt)
        {
            qDebug() << "no real URLs, skip";
            continue;
        }

        if (!urlsIt->isArray())
        {
            qDebug() << "real URLs is expected to be an array";
            continue;
        }

        auto urls = urlsIt->toArray();
        for (auto url : urls)
        {
            if (url.isString())
            {
                si->urls.push_back(url.toString());
            }
        }

        streams.push_back(si);
    }
}
