#include "annieprocess.h"
#include "config.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QStandardPaths>
#include <QNetworkProxy>

AnnieProcess::AnnieProcess(QObject *parent)
    : LinkResolverProcess (parent)
{

}

void AnnieProcess::parseNode(const QJsonObject &o, MediaInfoPtr mi)
{
    if (mi->site.isEmpty())
    {
        auto site = o["site"];
        if (site.isString())
        {
            mi->site = site.toString();
        }
    }

    if (mi->title.isEmpty())
    {
        auto title = o["title"];
        if (title.isString())
        {
            mi->title = title.toString();
        }
    }

    auto ss = o["streams"];
    if (!ss.isObject())
    {
        ss = o["formats"];
        if (!ss.isObject())
        {
            qDebug() << "Formats is expected to be an object";
            return;
        }
    }

    auto sso = ss.toObject();
    auto keys = sso.keys();

    for (const QString& key : keys)
    {
        StreamInfoPtr stream(new StreamInfo);
        auto streamObject = sso[key];
        auto s = streamObject.toObject();
        auto urlsArray = s["urls"];
        auto urls = urlsArray.toArray();
        if (urls.isEmpty())
            continue;
        for (auto url : urls)
        {
            auto urlObj = url.toObject();
            stream->urls.append(urlObj["url"].toString());
            stream->container = urlObj["ext"].toString();
        }
        stream->quality = s["quality"].toString();
        mi->annie.append(stream);
    }
}

void AnnieProcess::init()
{
    m_args.clear();
    m_args << "-j";
    Config cfg;
    setProgram(cfg.read<QString>("annie"));
}

void AnnieProcess::start(const QString &url)
{
    QStringList args;
    args << m_args;
    // network proxy
    if (needProxy(url))
    {
        Config cfg;
        if (cfg.read<int>(QLatin1String("proxyType")) == QNetworkProxy::Socks5Proxy)
            args << "-s" << QString("%1:%2").arg(cfg.read<QString>(QLatin1String("proxyHostName")))
                    .arg(cfg.read<int>(QLatin1String("proxyPort"), 1080));
        else
            args << "-x" << QString("%1:%2").arg(cfg.read<QString>(QLatin1String("proxyHostName")))
                    .arg(cfg.read<int>(QLatin1String("proxyPort"), 1080));
    }

    args << url;
    m_process.setArguments(args);

    LinkResolverProcess::start(url);
}
