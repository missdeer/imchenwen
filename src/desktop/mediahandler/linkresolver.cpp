#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QTextStream>

#include "linkresolver.h"

#include "browser.h"
#include "config.h"

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
    m_luxProcess.stop();
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
        m_mediaInfo->Reset();
        m_mediaInfo->url = url;

        m_youtubedlProcess.start(url);
        m_yougetProcess.start(url);
        m_ykdlProcess.start(url);
        m_luxProcess.start(url);

        m_lastUrl = url;
    }
}

void LinkResolver::onReadResolverOutput(const QByteArray &data)
{
    auto *p = qobject_cast<LinkResolverProcess *>(sender());
    p->resolved(m_mediaInfo);
    QJsonParseError e;
    QJsonDocument doc = QJsonDocument::fromJson(data, &e);
    if (e.error == QJsonParseError::IllegalUTF8String)
    {
        QTextStream in(data);
        auto d = in.readAll();
        doc = QJsonDocument::fromJson(d.toUtf8(), &e);
    }
    if (e.error != QJsonParseError::NoError)
    {
        qDebug() << __FUNCTION__ << __LINE__ << e.errorString() << QString(data);
    }

    if (doc.isObject())
    {
        p->parseNode(doc.object(), m_mediaInfo);
    }

    m_mediaInfo->resultCount++;

    if ((m_mediaInfo->ykdl.isEmpty() && m_mediaInfo->you_get.isEmpty() && m_mediaInfo->youtube_dl.isEmpty() && m_mediaInfo->lux.isEmpty()) ||
        (m_mediaInfo->title.isEmpty() && m_mediaInfo->site.isEmpty()))
    {
        if (m_mediaInfo->resultCount == 4) // currently there are 4 resolvers
        {
            emit error(m_lastUrl, tr("Resolving failed."));
            m_lastUrl.clear();
        }
        qDebug() << __FUNCTION__ << __LINE__ << m_lastUrl << "Resolving failed.";
    }

    if (!m_stopped)
    {
        qDebug() << __FUNCTION__ << __LINE__ << m_lastUrl << "Resolved.";
        emit done(m_lastUrl, m_mediaInfo);
    }
}

void LinkResolver::setupResolvers()
{
    disconnect(&m_luxProcess, &LinkResolverProcess::done, this, &LinkResolver::onReadResolverOutput);
    m_luxProcess.init();
    connect(&m_luxProcess, &LinkResolverProcess::done, this, &LinkResolver::onReadResolverOutput);

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
