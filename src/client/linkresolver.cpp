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
    , m_resolvers({
        { "you-get", &m_yougetProcess, std::bind(&LinkResolver::parseYouGetNode, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), &m_mediaInfo->you_get, QStringList() << "--json"  },
        { "ykdl", &m_ykdlProcess, std::bind(&LinkResolver::parseYKDLNode, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), &m_mediaInfo->ykdl, QStringList()  << "--json"},
        { "youtube-dl", &m_youtubedlProcess, std::bind(&LinkResolver::parseYoutubeDLNode, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), &m_mediaInfo->youtube_dl, QStringList()<< "--skip-download" << "--print-json"},
        { "annie", &m_annieProcess, std::bind(&LinkResolver::parseAnnieNode, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), &m_mediaInfo->annie, QStringList() << "-j"},
})
{
    Config cfg;
    for (auto & r : m_resolvers)
    {
        r.process->setProgram(cfg.read<QString>(r.name));
        connect(r.process, &LinkResolverProcess::data, this, &LinkResolver::readResolverOutput);
    }
}

void LinkResolver::resolve(const QString& url)
{
    if (url == m_lastUrl)
    {
        emit resolvingFinished(m_mediaInfo);
    }
    else
    {
        m_mediaInfo->title.clear();
        m_mediaInfo->site.clear();
        m_mediaInfo->resultCount = 0;
        m_mediaInfo->url = url;
        for ( auto & r : m_resolvers)
        {
            r.streams->clear();
            r.process->terminate();
            r.process->setArguments(QStringList() << r.args << url);
            r.process->start();
        }
        m_lastUrl = url;
    }
}

void LinkResolver::readResolverOutput(const QByteArray &data)
{
    LinkResolverProcess* p = qobject_cast<LinkResolverProcess*>(sender());

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    qDebug() << __FUNCTION__ << error.errorString();
    if (doc.isObject())
    {
        auto it = std::find_if(m_resolvers.begin(), m_resolvers.end(), [p](const auto &r ) {
            return r.process == p;
        });
        if (m_resolvers.end() != it)
            it->parse(doc.object(), m_mediaInfo, *it->streams);
    }

    m_mediaInfo->resultCount++;
    if (m_mediaInfo->resultCount == m_resolvers.length())
    {
        if (m_mediaInfo->title.isEmpty() && m_mediaInfo->site.isEmpty())
        {
            m_lastUrl.clear();
            emit resolvingError("Resolving failed.");
        }
        else
            emit resolvingFinished(m_mediaInfo);
    }
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
        stream->quality = format["video_profile"].toString();
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
