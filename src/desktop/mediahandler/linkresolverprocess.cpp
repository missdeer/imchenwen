#include "linkresolverprocess.h"
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

void LinkResolverProcess::setProgram(const QString &program)
{
    m_process.setProgram(program);
}

void LinkResolverProcess::setArguments(const QStringList &arguments)
{
    m_process.setArguments(arguments);
}

void LinkResolverProcess::stop()
{
    if (m_process.state() == QProcess::Running)
        m_process.kill();
}

void LinkResolverProcess::start()
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

void LinkResolverProcess::setTimeout(int timeout)
{
    m_timeout = timeout;
}
