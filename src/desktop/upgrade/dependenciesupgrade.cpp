#include "dependenciesupgrade.h"
#include "networkreplyhelper.h"
#include "config.h"
#include <QStringList>
#include <QFile>
#include <QFileInfo>
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
    auto appLocalDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir d(appLocalDataPath);
    if (!d.exists())
        d.mkpath(appLocalDataPath);
#if defined(Q_OS_WIN64)
    QString ffmpegUrl = "https://ffmpeg.zeranoe.com/builds/win64/static/ffmpeg-latest-win64-static.zip";
    QString ffmpeg = appLocalDataPath + "/ffmpeg-latest-win64-static/bin/ffmpeg.exe";

    // https://github.com/iawia002/annie/releases/download/0.9.0/annie_0.9.0_Windows_64-bit.zip
#else
    QString ffmpegUrl = "https://ffmpeg.zeranoe.com/builds/win32/static/ffmpeg-latest-win32-static.zip";
    QString ffmpeg = appLocalDataPath + "/ffmpeg-latest-win32-static/bin/ffmpeg.exe";

    // https://github.com/iawia002/annie/releases/download/0.9.0/annie_0.9.0_Windows_32-bit.zip
#endif
    if (QFile::exists(ffmpeg))
    {
        // download latest ffmpeg zip package
    }
    QString annieReleaseUrl = "https://github.com/iawia002/annie/releases/latest";

    QString python = appLocalDataPath + "/python/python.exe";
    QString pip3 = appLocalDataPath + "/python/Scripts/pip3.exe";

    Config cfg;
    QString ykdlPath = cfg.read<QString>("ykdl");
    QFileInfo fi(ykdlPath);
    if (fi.absoluteDir() != QDir(appLocalDataPath + "/Scripts"))
    {
        if (QFile::exists(fi.absolutePath() + "/pip3.exe"))
        {
            pip3 = fi.absolutePath() + "/pip3.exe";
        }
        QDir d = fi.absoluteDir();
        d.cdUp();
        if (QFile::exists(d.absolutePath() + "/python.exe"))
        {
            python = d.absolutePath() + "/python.exe";
        }
    }

    if (QFile::exists(python) && QFile::exists(pip3))
    {
        QProcess process;
        process.setProgram(python);
        process.setArguments(QStringList() << QDir::toNativeSeparators(pip3) << "install" << "--upgrade" << "ykdl" << "you-get" << "youtube-dl");
        process.setWorkingDirectory(QFileInfo(pip3).absolutePath());
        process.start();
        process.waitForFinished();
    }
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
    process.start("/usr/local/bin/brew upgrade mpv annie python3");
    process.waitForFinished();
    // pip3 install --upgrade ykdl you-get
    process.start("/usr/local/bin/pip3 install --upgrade ykdl you-get");
    process.waitForFinished();
}
