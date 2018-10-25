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
    connect(&m_yougetProcess, &LinkResolverProcess::data, this, &LinkResolver::readResolverOutput);

    m_ykdlProcess.setProgram(cfg.read<QString>("ykdl"));
    connect(&m_ykdlProcess, &LinkResolverProcess::data, this, &LinkResolver::readResolverOutput);

    m_youtubedlProcess.setProgram(cfg.read<QString>("youtube-dl"));
    connect(&m_youtubedlProcess, &LinkResolverProcess::data, this, &LinkResolver::readResolverOutput);

    m_annieProcess.setProgram(cfg.read<QString>("annie"));
    connect(&m_annieProcess, &LinkResolverProcess::data, this, &LinkResolver::readResolverOutput);
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

        struct {
            LinkResolverProcess* p;
            Streams*  ss;
            QStringList args;
        } resolvers[] = {
        { &m_yougetProcess, &m_mediaInfo->you_get, QStringList() << "--json" << url },
        { &m_ykdlProcess, &m_mediaInfo->ykdl, QStringList()  << "--json" << url},
        { &m_youtubedlProcess, &m_mediaInfo->youtube_dl, QStringList()<< "--skip-download" << "--print-json" << url },
        { &m_annieProcess, &m_mediaInfo->annie, QStringList() << "-j" << url },
        };
        for ( auto & r : resolvers)
        {
            r.ss->clear();
            r.p->terminate();
            r.p->setArguments(r.args);
            r.p->start();
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
        struct {
            LinkResolverProcess* p;
            std::function<void(const QJsonObject , MediaInfoPtr , Streams &)> f;
            Streams& ss;
        } parsers []{
        { &m_yougetProcess, std::bind(&LinkResolver::parseYouGetNode, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), m_mediaInfo->you_get},
        { &m_ykdlProcess, std::bind(&LinkResolver::parseYKDLNode, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), m_mediaInfo->ykdl},
        { &m_youtubedlProcess, std::bind(&LinkResolver::parseYoutubeDLNode, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), m_mediaInfo->youtube_dl},
        { &m_annieProcess, std::bind(&LinkResolver::parseAnnieNode, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), m_mediaInfo->annie},
        };

        auto it = std::find_if(std::begin(parsers), std::end(parsers), [p](const auto &parser ) {
            return parser.p == p;
        });
        if (std::end(parsers) != it)
            it->f(doc.object(),m_mediaInfo, it->ss);
    }

    m_mediaInfo->resultCount++;
    if (m_mediaInfo->resultCount == 4)
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
