#include "youtubedlprocess.h"
#include "config.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QStandardPaths>
#include <QNetworkProxy>

YoutubeDLProcess::YoutubeDLProcess(QObject *parent)
    : LinkResolverProcess (parent)
{

}

void YoutubeDLProcess::parseNode(const QJsonObject &o, MediaInfoPtr mi)
{
    if (mi->site.isEmpty())
    {
        auto site = o["extractor_key"];
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

    auto formats = o["formats"];
    if (!formats.isArray())
    {
        qDebug() << "Formats is expected to be an array";
        return;
    }
    auto formatsArray = formats.toArray();
    for (auto fo : formatsArray)
    {
        StreamInfoPtr stream(new StreamInfo);
        auto format = fo.toObject();
        stream->quality = format["format"].toString();
        stream->container = format["ext"].toString();
        stream->urls.append(format["url"].toString());
        mi->youtube_dl.append(stream);
    }

    auto subtitles = o["subtitles"];
    parseSubtitle(subtitles, mi, true);

    auto automaticCaptions = o["automatic_captions"];
    parseSubtitle(automaticCaptions, mi, false);
}

void YoutubeDLProcess::init()
{
    m_args << "--skip-download" << "--print-json" << "--no-warnings" << "--no-playlist" << "--flat-playlist" << "--sub-lang" << "zh-CN" << "--write-auto-sub" << "--write-sub";
    Config cfg;
    auto appLocalDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

#if defined(Q_OS_WIN)
    QString path = cfg.read<QString>("youtube-dl");
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
    setProgram(cfg.read<QString>("youtube-dl"));
#endif
    setTimeout(30 * 1000);
}

void YoutubeDLProcess::start(const QString &url)
{
    QStringList args;
    args << m_args;
    // network proxy
    if (needProxy(url))
    {
        Config cfg;
        if (cfg.read<int>(QLatin1String("proxyType")) == QNetworkProxy::Socks5Proxy)
            args << "--proxy" << QString("socks5://%1:%2").arg(cfg.read<QString>(QLatin1String("proxyHostName")))
                    .arg(cfg.read<int>(QLatin1String("proxyPort"), 1080));
        else
            args << "--proxy" << QString("http://%1:%2").arg(cfg.read<QString>(QLatin1String("proxyHostName")))
                    .arg(cfg.read<int>(QLatin1String("proxyPort"), 1080));
    }

    args << url;
    m_process.setArguments(args);

    LinkResolverProcess::start(url);
}

void YoutubeDLProcess::parseSubtitle(const QJsonValue &v, MediaInfoPtr mi, bool manual)
{
    if (!v.isObject())
        return;

    auto obj = v.toObject();
    QStringList languages = obj.keys();
    for (const auto& lang : languages)
    {
        auto langNode = obj[lang].toArray();
        auto it = std::find_if(langNode.begin(), langNode.end(), [](QJsonValueRef langFile){
                    if (!langFile.isObject())
                        return false;
                    auto langFileObj = langFile.toObject();
                    return (langFileObj["ext"].isString() && langFileObj["ext"].toString() == "vtt");
        });
        if (langNode.end() != it)
        {
            auto langFileObj = it->toObject();
            SubtitlePtr subtitle(new Subtitle);
            subtitle->url = langFileObj["url"].toString();
            subtitle->language = lang;
            subtitle->manual = manual;
            mi->subtitles.append(subtitle);
        }
    }
}
