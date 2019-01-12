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
    m_args << "--json";
    Config cfg;
    auto appLocalDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

#if defined(Q_OS_WIN)
    QString path = cfg.read<QString>("you-get");
    QFileInfo fi(path);
    QDir dir(appLocalDataPath + "/python/Scripts");
    if (fi.absoluteDir() == dir)
    {
        QString python = appLocalDataPath + "/python/python.exe";
        setProgram(python);
        if (m_args.at(0) != QDir::toNativeSeparators(path))
            m_args.insert(0, QDir::toNativeSeparators(path));
    }
    else
    {
        setProgram(path);
    }
#else
    setProgram(cfg.read<QString>("you-get"));
#endif
}
