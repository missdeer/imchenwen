#include "linkresolverprocess.h"
#include "browser.h"
#include "config.h"
#include "outofchinamainlandproxyfactory.h"
#include "ingfwlistproxyfactory.h"
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
        int scope = cfg.read<int>(QLatin1String("proxyScope"));
        switch (scope)
        {
        case 0:
            return true;
        case 1:
            return Browser::instance().outOfChinaMainlandProxyFactory()->needProxy(url);
        case 2:
            return Browser::instance().inGFWListProxyFactory()->needProxy(url);
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
        "hdflv",
        "720p60",
        "720p H265",
        "1080p",
        "1080p60",
        "1440p",
        "2160p",
        "4320p",
        "4k"
    };
    auto it = std::find_if(keywords.rbegin(), keywords.rend(), [lhs](const QString& keyword){
        return lhs->quality.contains(keyword, Qt::CaseInsensitive);
    });
    if (keywords.rend() == it)
        return true;
    auto lhsIndex = std::distance(keywords.rbegin(), it);
    it = std::find_if(keywords.rbegin(), keywords.rend(), [rhs](const QString& keyword){
        return rhs->quality.contains(keyword, Qt::CaseInsensitive);
    });
    if (keywords.rend() == it)
        return false;
    auto rhsIndex = std::distance(keywords.rbegin(), it);
    return lhsIndex > rhsIndex;
}
