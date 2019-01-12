#include "linkresolverprocess.h"
#include "browser.h"
#include "config.h"
#include <QUrl>
#include <QTimer>
#include <QStandardPaths>

LinkResolverProcess::LinkResolverProcess(QObject *parent)
    : QObject(parent)
    , m_timeout(15 * 1000)
{
    m_process.setProcessChannelMode(QProcess::MergedChannels);
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &LinkResolverProcess::onReadStandardOutput);
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &LinkResolverProcess::onFinished);

    m_process.setWorkingDirectory( QStandardPaths::writableLocation(QStandardPaths::TempLocation));
}

LinkResolverProcess::~LinkResolverProcess()
{
    disconnect(&m_process, &QProcess::readyReadStandardOutput, this, &LinkResolverProcess::onReadStandardOutput);
    disconnect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &LinkResolverProcess::onFinished);
}

void LinkResolverProcess::setProgram(const QString &program)
{
    m_process.setProgram(program);
}

void LinkResolverProcess::stop()
{
    if (m_process.state() == QProcess::Running)
        m_process.kill();
}

void LinkResolverProcess::start(const QString& )
{
    m_data.clear();
    m_process.start();
    QTimer::singleShot(m_timeout, this, &LinkResolverProcess::stop);
}

void LinkResolverProcess::onReadStandardOutput()
{
    m_data.append(m_process.readAll());
}

void LinkResolverProcess::onFinished(int , QProcess::ExitStatus )
{
    emit done(m_data);
}

bool LinkResolverProcess::needProxy(const QString &url)
{
    Config cfg;
    if (cfg.read<bool>(QLatin1String("enableProxy"), false) && cfg.read<bool>(QLatin1String("applyProxyToResolvers"), true))
    {
        if ((cfg.read<bool>(QLatin1String("applyProxyAbroadOnly"), true)
             && !Browser::instance().shortcuts().isIn(QUrl(url), "china"))
                || !cfg.read<bool>(QLatin1String("applyProxyAbroadOnly"), true))
        {
            return true;
        }
    }
    return false;
}

void LinkResolverProcess::setTimeout(int timeout)
{
    m_timeout = timeout;
}

void LinkResolverProcess::parseNode(const QJsonObject &, MediaInfoPtr )
{
    // implement in sub-class
}

void LinkResolverProcess::init()
{
    // implement in sub-class
}

bool operator<(StreamInfoPtr lhs, StreamInfoPtr rhs)
{
    QStringList keywords = {
        "144p",
        "210p",
        "240p",
        "640x360",
        "360p",
        "480p",
        "540p",
        "540p H265",
        "640p",
        "1280x720",
        "hd720",
        "720p",
        "720p H265",
        "1080p",
        "1440p",
        "2160p",
        "4320p",
        "4k"
    };
    auto it = std::find_if(keywords.begin(), keywords.end(), [lhs](const QString& keyword){
        return lhs->quality.contains(keyword);
    });
    if (keywords.end() == it)
        return true;
    auto lhsIndex = std::distance(keywords.begin(), it);
    it = std::find_if(keywords.begin(), keywords.end(), [rhs](const QString& keyword){
        return rhs->quality.contains(keyword);
    });
    if (keywords.end() == it)
        return false;
    auto rhsIndex = std::distance(keywords.begin(), it);
    return lhsIndex < rhsIndex;
}
