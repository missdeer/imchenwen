#include "ykdlprocess.h"
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

YKDLProcess::YKDLProcess(QObject *parent)
    :LinkResolverProcess (parent)
{

}

void YKDLProcess::parseNode(const QJsonObject &o, MediaInfoPtr mi)
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
        qDebug() << "streams is expected to be an object";
        return;
    }

    auto so = ss.toObject();
    auto keys = so.keys();
    for ( const QString& key : keys)
    {
        StreamInfoPtr stream(new StreamInfo);
        auto formatObject = so[key];
        auto format = formatObject.toObject();
        auto urlsArray = format["src"];
        auto urls = urlsArray.toArray();
        if (urls.isEmpty())
            continue;
        for (auto url : urls)
        {
            stream->urls.append(url.toString());
        }
        stream->container = format["container"].toString();
        stream->quality = format["video_profile"].toString();
        mi->ykdl.append(stream);
    }
}

void YKDLProcess::init()
{
    m_args << "--json";
    Config cfg;
    auto appLocalDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

#if defined(Q_OS_WIN)
    QString path = cfg.read<QString>("ykdl");
    QFileInfo fi(path);
    QDir d(fi.absoluteDir());
    d.cdUp();
    QString python = d.absolutePath() + "/python.exe";
    if (QFile::exists(python))
    {
        setProgram(python);
        if (m_args.at(0) != QDir::toNativeSeparators(path))
            m_args.insert(0, QDir::toNativeSeparators(path));
    }
    else
    {
        setProgram(path);
    }
#else
    setProgram(cfg.read<QString>("ykdl"));
#endif
}

void YKDLProcess::start(const QString &url)
{
    QStringList args;
    args << m_args;
    // network proxy
    if (needProxy(url))
    {
        Config cfg;
        if (cfg.read<int>(QLatin1String("proxyType")) == QNetworkProxy::HttpProxy)
        {
            args << "--proxy" << QString("%1:%2").arg(cfg.read<QString>(QLatin1String("proxyHostName")))
                    .arg(cfg.read<int>(QLatin1String("proxyPort"), 1080));
        }
    }

    args << url;
    m_process.setArguments(args);

    LinkResolverProcess::start(url);
}
