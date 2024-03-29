#include "sniffer.h"
#include <QApplication>
#include <QDir>

Sniffer::Sniffer(QObject *parent)
    : QObject(parent)
{
    QDir dir(qApp->applicationDirPath());
#if defined(Q_OS_MAC)
    dir.cdUp();
    dir.cd("Tools");
#endif

#if defined(Q_OS_WIN)
    QString filePath = dir.absolutePath() + "/sniff.exe";
#else
    QString filePath = dir.absolutePath() + "/sniff";
#endif
    m_process.setProgram(filePath);
}

void Sniffer::sniff(const QString &url)
{
    m_process.setProcessChannelMode(QProcess::SeparateChannels);
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &Sniffer::onReadStandardOutput);
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Sniffer::onFinished);
    m_originalUrl = url;
    m_data.clear();
    m_process.setArguments(QStringList() << url);
    m_process.start();
}

void Sniffer::stop()
{
    //m_process.kill();
    disconnect(&m_process, &QProcess::readyReadStandardOutput, this, &Sniffer::onReadStandardOutput);
    disconnect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Sniffer::onFinished);
    emit error();
}

void Sniffer::onReadStandardOutput()
{
    m_data.append(m_process.readAll());
}

void Sniffer::onFinished(int exitCode, QProcess::ExitStatus )
{
    m_data.append(m_process.readAll());
    if (exitCode != 0)
        emit error();
    else
    {
        auto lines = m_data.split('\n');
        for (const auto & line : lines)
        {
            if (line.trimmed().startsWith("http://") || line.trimmed().startsWith("https://"))
                emit done(m_originalUrl, QString(line.trimmed()));
        }
    }
}
