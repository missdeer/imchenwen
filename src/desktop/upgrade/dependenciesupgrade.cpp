#include "dependenciesupgrade.h"
#include "networkreplyhelper.h"
#include <QStringList>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>

DependenciesUpgrade::DependenciesUpgrade()
{

}

void DependenciesUpgrade::run()
{
#if defined (Q_OS_WIN)
    upgradeForWin();
#elif defined(Q_OS_MAC)
    upgradeForMac();
#else
#endif
}

void DependenciesUpgrade::upgradeForWin()
{
#if defined(Q_OS_WIN64)
    // https://ffmpeg.zeranoe.com/builds/win64/static/ffmpeg-latest-win64-static.zip

    // https://github.com/iawia002/annie/releases/latest
    // https://github.com/iawia002/annie/releases/download/0.9.0/annie_0.9.0_Windows_64-bit.zip
#else
    // https://ffmpeg.zeranoe.com/builds/win32/static/ffmpeg-latest-win32-static.zip

    // https://github.com/iawia002/annie/releases/latest
    // https://github.com/iawia002/annie/releases/download/0.9.0/annie_0.9.0_Windows_32-bit.zip
#endif
    auto appLocalDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir d(appLocalDataPath);
    if (!d.exists())
        d.mkpath(appLocalDataPath);
    QString python = appLocalDataPath + "/python/pythonw.exe";
    QString pip3 = appLocalDataPath + "/python/Scripts/pip3.exe";
    QProcess process;
    process.setProgram(python);
    process.setArguments(QStringList() << QDir::toNativeSeparators(pip3) << "install" << "--upgrade" << "ykdl" << "you-get" << "youtube-dl");
    process.setWorkingDirectory(appLocalDataPath + "/python/Scripts");
    process.start();
    process.waitForFinished();
}

void DependenciesUpgrade::upgradeForMac()
{
    QStringList fromBrew{
        "/usr/local/bin/mpv",
        "/usr/local/bin/annie",
        "/usr/local/bin/ffmpeg",
        "/usr/local/bin/youtube-dl",
        "/usr/local/bin/python3",
        "/usr/local/bin/pip3",
    };
    QProcess process;
    for (const auto & p : fromBrew)
    {
        if (!QFile::exists(p))
        {
            // brew install mpv annie ffmpeg youtube-dl python3 pip3
            process.start("/usr/local/bin/brew install " + p.split(QChar('/'))[3]);
            process.waitForFinished();
        }
    }
    // brew update && brew upgrade
    process.start("/usr/local/bin/brew update");
    process.waitForFinished();
    process.start("/usr/local/bin/brew upgrade");
    process.waitForFinished();
    // pip3 install --upgrade ykdl you-get
    process.start("/usr/local/bin/pip3 install --upgrade ykdl you-get");
    process.waitForFinished();
}
