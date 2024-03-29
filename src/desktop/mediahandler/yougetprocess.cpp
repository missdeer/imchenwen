#include "yougetprocess.h"
#include "config.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QStandardPaths>
#include <QNetworkProxy>

YouGetProcess::YouGetProcess(QObject *parent)
    : LinkResolverProcess (parent)
{

}

void YouGetProcess::parseNode(const QJsonObject &o, MediaInfoPtr mi)
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
        qDebug() << "Formats is expected to be an object";
        return;
    }

    auto s = ss.toObject();
    auto keys = s.keys();
    for ( const QString& key : keys)
    {
        StreamInfoPtr stream(new StreamInfo);
        auto formatObject = s[key];
        auto format = formatObject.toObject();
        auto urlsArray = format["src"];
        auto urls = urlsArray.toArray();
        if (urls.isEmpty())
            continue;
        for (auto url : urls)
        {
            stream->urls.append(url.toString());
        }
        stream->quality = format["video_profile"].toString();
        if (stream->quality.isEmpty())
            stream->quality = key;
        stream->container = format["container"].toString();
        mi->you_get.append(stream);
    }
}

void YouGetProcess::init()
{
    m_args.clear();
    m_args << "--json";
    Config cfg;
    auto appLocalDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

    setProgram(cfg.read<QString>("you-get"));
}

void YouGetProcess::start(const QString &url)
{
    QStringList args;
    args << m_args;
    // network proxy
    if (needProxy(url))
    {
        Config cfg;
        if (cfg.read<int>(QLatin1String("proxyType")) == QNetworkProxy::HttpProxy)
        {
            args << "-y" << QString("%1:%2").arg(cfg.read<QString>(QLatin1String("proxyHostName")))
                    .arg(cfg.read<int>(QLatin1String("proxyPort"), 1080));
        }
    }

    args << url;
    m_process.setArguments(args);

    LinkResolverProcess::start(url);
}

void YouGetProcess::resolved(MediaInfoPtr mi)
{
    mi->you_getDone = true;
}
