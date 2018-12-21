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
    setupResolvers();
}

void LinkResolver::terminateResolvers()
{
    m_yougetProcess.stop();
    m_ykdlProcess.stop();
    m_youtubedlProcess.stop();
    m_annieProcess.stop();
}

LinkResolver::~LinkResolver()
{
    terminateResolvers();
}

void LinkResolver::resolve(const QString& url)
{
    if (url == m_lastUrl)
    {
        emit done(m_lastUrl, m_mediaInfo);
    }
    else
    {
        terminateResolvers();
        m_mediaInfo->title.clear();
        m_mediaInfo->site.clear();

        m_mediaInfo->resultCount = 0;
        m_mediaInfo->url = url;
        for ( auto & r : m_resolvers)
        {
            r.streams->clear();
            r.process->setArguments(QStringList() << r.args << url);
            r.process->start();
        }
        m_lastUrl = url;
    }
}

void LinkResolver::onReadResolverOutput(const QByteArray &data)
{
    LinkResolverProcess *p = qobject_cast<LinkResolverProcess*>(sender());

    QJsonParseError e;
    QJsonDocument doc = QJsonDocument::fromJson(data, &e);
    if (e.error == QJsonParseError::IllegalUTF8String)
    {
        QTextStream in(data);
        auto d = in.readAll();
        doc = QJsonDocument::fromJson(d.toUtf8(), &e);
    }
    if (e.error != QJsonParseError::NoError)
        qDebug() << __FUNCTION__ << e.errorString() << QString(data);

    if (doc.isObject())
    {
        auto it = std::find_if(m_resolvers.begin(), m_resolvers.end(), [p](const Resolver &r ) {
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
            emit error(m_lastUrl, tr("Resolving failed."));
        }
        else
            emit done(m_lastUrl, m_mediaInfo);
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
        if (urls.isEmpty())
            continue;
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
        streams.append(stream);
    }
}

void LinkResolver::setupResolvers()
{
    Config cfg;
    auto appLocalDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QString python = appLocalDataPath + "/python/pythonw.exe";
    QDir dir(appLocalDataPath + "/python/Scripts");
    for (auto & r : m_resolvers)
    {
        disconnect(r.process, &LinkResolverProcess::done, this, &LinkResolver::onReadResolverOutput);
#if defined(Q_OS_WIN)
        QString path = cfg.read<QString>(r.name);
        QFileInfo fi(path);
        if (fi.absoluteDir() == dir)
        {
            r.process->setProgram(python);
            if (r.args.at(0) != QDir::toNativeSeparators(path))
                r.args.insert(0, QDir::toNativeSeparators(path));
        }
        else
        {
            r.process->setProgram(path);
        }
#else
        r.process->setProgram(cfg.read<QString>(r.name));
#endif
        connect(r.process, &LinkResolverProcess::done, this, &LinkResolver::onReadResolverOutput);
    }
}
