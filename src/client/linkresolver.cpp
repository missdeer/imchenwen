#include "linkresolver.h"
#include "browser.h"
#include "config.h"
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QTextStream>

static QNetworkAccessManager nam;

LinkResolver::LinkResolver(QObject *parent)
    : QObject(parent)
    , m_mediaInfo(new MediaInfo)
{
    Config cfg;
    m_yougetProcess.setProgram(cfg.read<QString>("you-get"));
    connect(&m_yougetProcess, &LinkResolverProcess::data, this, &LinkResolver::readYouGetOutput);

    m_ykdlProcess.setProgram(cfg.read<QString>("ykdl"));
    connect(&m_ykdlProcess, &LinkResolverProcess::data, this, &LinkResolver::readYKDLOutput);

    m_youtubedlProcess.setProgram(cfg.read<QString>("youtube-dl"));
    connect(&m_youtubedlProcess, &LinkResolverProcess::data, this, &LinkResolver::readYoutubeDLOutput);

    m_annieProcess.setProgram(cfg.read<QString>("annie"));
    connect(&m_annieProcess, &LinkResolverProcess::data, this, &LinkResolver::readAnnieOutput);
}

void LinkResolver::resolve(const QString& url)
{
    m_mediaInfo->resultCount = 0;
    resolveByYouGet(url);
    resolveByYKDL(url);
    resolveByYoutubeDL(url);
    resolveByAnnie(url);
}

void LinkResolver::resolveVIP(const QString &url)
{
}

void LinkResolver::readYouGetOutput(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    qDebug() << __FUNCTION__ << error.errorString();
    if (doc.isObject())
        parseYouGetNode(doc.object(), m_mediaInfo, m_mediaInfo->you_get);

    m_mediaInfo->resultCount++;
    if (m_mediaInfo->resultCount == 4)
        emit resolvingFinished(m_mediaInfo);
}

void LinkResolver::readYKDLOutput(const QByteArray &data)
{
    QJsonParseError error;
    QTextStream in(data);
    QJsonDocument doc = QJsonDocument::fromJson(in.readAll().toUtf8(), &error);
    qDebug() << __FUNCTION__ << error.errorString();
    if (doc.isObject())
        parseYKDLNode(doc.object(), m_mediaInfo, m_mediaInfo->ykdl);

    m_mediaInfo->resultCount++;
    if (m_mediaInfo->resultCount == 4)
        emit resolvingFinished(m_mediaInfo);
}

void LinkResolver::readYoutubeDLOutput(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    qDebug() << __FUNCTION__ << error.errorString();
    if (doc.isObject())
        parseYoutubeDLNode(doc.object(), m_mediaInfo, m_mediaInfo->youtube_dl);

    m_mediaInfo->resultCount++;
    if (m_mediaInfo->resultCount == 4)
        emit resolvingFinished(m_mediaInfo);
}

void LinkResolver::readAnnieOutput(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    qDebug() << __FUNCTION__ << error.errorString();
    if (doc.isObject())
        parseAnnieNode(doc.object(), m_mediaInfo, m_mediaInfo->annie);

    m_mediaInfo->resultCount++;
    if (m_mediaInfo->resultCount == 4)
        emit resolvingFinished(m_mediaInfo);
}

void LinkResolver::parseYouGetNode(const QJsonObject &o, MediaInfoPtr mi, Streams &streams)
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
        for (auto url : urls)
        {
            stream->urls.append(url.toString());
        }
        stream->container = format["container"].toString();
        streams.append(stream);
    }
}

void LinkResolver::parseYKDLNode(const QJsonObject &o, MediaInfoPtr mi, Streams &streams)
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
        for (auto url : urls)
        {
            stream->urls.append(url.toString());
        }
        stream->container = format["container"].toString();
        stream->quality = format["video_profile"].toString();
        streams.append(stream);
    }
}

void LinkResolver::parseYoutubeDLNode(const QJsonObject &o, MediaInfoPtr mi, Streams &streams)
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
        streams.append(stream);
    }
}

void LinkResolver::parseAnnieNode(const QJsonObject &o, MediaInfoPtr mi, Streams &streams)
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

    auto formats = o["Formats"];
    if (!formats.isObject())
    {
        qDebug() << "Formats is expected to be an object";
        return;
    }

    auto fo = formats.toObject();
    auto keys = fo.keys();

    for (const QString& key : keys)
    {
        StreamInfoPtr stream(new StreamInfo);
        auto formatObject = fo[key];
        auto format = formatObject.toObject();
        auto urlsArray = format["URLs"];
        auto urls = urlsArray.toArray();
        for (auto url : urls)
        {
            auto urlObj = url.toObject();
            stream->urls.append(urlObj["URL"].toString());
        }
        stream->quality = format["Quality"].toString();
        streams.append(stream);
    }
}

void LinkResolver::resolveByYouGet(const QString &url)
{
    m_mediaInfo->you_get.clear();
    m_yougetProcess.terminate();

    m_yougetProcess.setArguments(QStringList() << "--json" << url);
    m_yougetProcess.start();
}

void LinkResolver::resolveByYKDL(const QString &url)
{
    m_mediaInfo->ykdl.clear();
    m_ykdlProcess.terminate();

    m_ykdlProcess.setArguments(QStringList() << "--json" << url);
    m_ykdlProcess.start();
}

void LinkResolver::resolveByYoutubeDL(const QString &url)
{
    m_mediaInfo->youtube_dl.clear();
    m_youtubedlProcess.terminate();

    m_youtubedlProcess.setArguments(QStringList() << "--skip-download" << "--print-json" << url);
    m_youtubedlProcess.start();
}

void LinkResolver::resolveByAnnie(const QString &url)
{
    m_mediaInfo->annie.clear();
    m_annieProcess.terminate();

    m_annieProcess.setArguments(QStringList() << "-j" << url);
    m_annieProcess.start();
}
