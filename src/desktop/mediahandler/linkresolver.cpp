#include "linkresolver.h"
#include "browser.h"
#include "config.h"
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QTextStream>

LinkResolver::LinkResolver(QObject *parent)
    : QObject(parent)
    , m_stopped(false)
    , m_mediaInfo(new MediaInfo)
{
    setupResolvers();
}

void LinkResolver::terminateResolvers()
{
    m_stopped = true;
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
        m_stopped = false;
        m_mediaInfo->title.clear();
        m_mediaInfo->site.clear();
        m_mediaInfo->subtitles.clear();
        m_mediaInfo->annie.clear();
        m_mediaInfo->ykdl.clear();
        m_mediaInfo->you_get.clear();
        m_mediaInfo->youtube_dl.clear();
        m_mediaInfo->resultCount = 0;
        m_mediaInfo->url = url;

        m_youtubedlProcess.start(url);
        m_yougetProcess.start(url);
        m_ykdlProcess.start(url);
        m_annieProcess.start(url);

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
        p->parseNode(doc.object(), m_mediaInfo);
    }

    m_mediaInfo->resultCount++;

    if ((m_mediaInfo->ykdl.isEmpty() &&
         m_mediaInfo->you_get.isEmpty() &&
         m_mediaInfo->youtube_dl.isEmpty() &&
         m_mediaInfo->annie.isEmpty()) ||
            (m_mediaInfo->title.isEmpty() &&
             m_mediaInfo->site.isEmpty()))
    {
        if (m_mediaInfo->resultCount == 4) // currently there are 4 resolvers
        {
            emit error(m_lastUrl, tr("Resolving failed."));
            m_lastUrl.clear();
        }
        return;
    }

    if (!m_stopped &&
            (!m_mediaInfo->title.isEmpty() ||
             !m_mediaInfo->site.isEmpty() ||
             !m_mediaInfo->ykdl.isEmpty() ||
             !m_mediaInfo->you_get.isEmpty() ||
             !m_mediaInfo->youtube_dl.isEmpty() ||
             !m_mediaInfo->annie.isEmpty()))
        emit done(m_lastUrl, m_mediaInfo);
}

void LinkResolver::setupResolvers()
{
    disconnect(&m_annieProcess, &LinkResolverProcess::done, this, &LinkResolver::onReadResolverOutput);
    m_annieProcess.init();
    connect(&m_annieProcess, &LinkResolverProcess::done, this, &LinkResolver::onReadResolverOutput);

    disconnect(&m_ykdlProcess, &LinkResolverProcess::done, this, &LinkResolver::onReadResolverOutput);
    m_ykdlProcess.init();
    connect(&m_ykdlProcess, &LinkResolverProcess::done, this, &LinkResolver::onReadResolverOutput);

    disconnect(&m_youtubedlProcess, &LinkResolverProcess::done, this, &LinkResolver::onReadResolverOutput);
    m_youtubedlProcess.init();
    connect(&m_youtubedlProcess, &LinkResolverProcess::done, this, &LinkResolver::onReadResolverOutput);

    disconnect(&m_yougetProcess, &LinkResolverProcess::done, this, &LinkResolver::onReadResolverOutput);
    m_yougetProcess.init();
    connect(&m_yougetProcess, &LinkResolverProcess::done, this, &LinkResolver::onReadResolverOutput);
}
