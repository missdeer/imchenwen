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
