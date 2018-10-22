#include "linkresolverprocess.h"

LinkResolverProcess::LinkResolverProcess(QObject *parent) : QObject(parent)
{
    m_process.setProcessChannelMode(QProcess::MergedChannels);
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &LinkResolverProcess::readStandardOutput);
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &LinkResolverProcess::finished);
}

void LinkResolverProcess::setProgram(const QString &program)
{
    m_process.setProgram(program);
}

void LinkResolverProcess::setArguments(const QStringList &arguments)
{
    m_process.setArguments(arguments);
}

void LinkResolverProcess::terminate()
{
    if (m_process.state() == QProcess::Running)
        m_process.terminate();
}

void LinkResolverProcess::start()
{
    m_data.clear();
    m_process.start();
}

void LinkResolverProcess::readStandardOutput()
{
    m_data.append(m_process.readAll());
}

void LinkResolverProcess::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    emit data(m_data);
}
