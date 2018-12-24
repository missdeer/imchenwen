#include "dependenciesupgrade.h"
#include "networkreplyhelper.h"
#include "config.h"
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QRegularExpression>
#include <private/qzipreader_p.h>
#include <private/qzipwriter_p.h>

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

    // install ykdl/you-get/youtube-dl via pip3
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
        process.setWorkingDirectory(fi.absolutePath());
        process.start();
        process.waitForFinished();

        if (!QFile::exists(cfg.read<QString>("ykdl")))
            cfg.write("ykdl", fi.absolutePath() + "/ykdl.exe");
        if (!QFile::exists(cfg.read<QString>("you-get")))
            cfg.write("you-get", fi.absolutePath() + "/you-get.exe");
        if (!QFile::exists(cfg.read<QString>("youtube-dl")))
            cfg.write("youtube-dl", fi.absolutePath() + "/youtube-dl.exe");
        if (!QFile::exists(cfg.read<QString>("annie")))
            cfg.write("annie", fi.absolutePath() + "/annie.exe");
    }

    // get ffmpeg & annie via http link
#if defined(Q_OS_WIN64)
    QString ffmpegUrl = "https://ffmpeg.zeranoe.com/builds/win64/static/ffmpeg-latest-win64-static.zip";
    QString ffmpeg = appLocalDataPath + "/ffmpeg-latest-win64-static/bin/ffmpeg.exe";

    // https://github.com/iawia002/annie/releases/download/0.9.0/annie_0.9.0_Windows_64-bit.zip
    QRegularExpression reg("\\/iawia002\\/annie\\/releases\\/download\\/[0-9\\.]+\\/annie_[0-9\\.]+_Windows_64\\-bit.zip");
#else
    QString ffmpegUrl = "https://ffmpeg.zeranoe.com/builds/win32/static/ffmpeg-latest-win32-static.zip";
    QString ffmpeg = appLocalDataPath + "/ffmpeg-latest-win32-static/bin/ffmpeg.exe";

    // https://github.com/iawia002/annie/releases/download/0.9.0/annie_0.9.0_Windows_32-bit.zip
    QRegularExpression reg("\\/iawia002\\/annie\\/releases\\/download\\/[0-9\\.]+\\/annie_[0-9\\.]+_Windows_32\\-bit.zip");
#endif
    QEventLoop loop;
    if (QFile::exists(ffmpeg) || !QFile::exists(cfg.read<QString>("ffmpeg")))
    {
        getFile(ffmpegUrl, appLocalDataPath + "/ffmpeg.zip");
        extractZIP(appLocalDataPath + "/ffmpeg.zip");
        QFile::remove(appLocalDataPath + "/ffmpeg.zip");
        cfg.write("ffmpeg", ffmpeg);
    }

    QString annie = d.absolutePath() + "/annie.exe";
    if (QFile::exists(annie) || !QFile::exists(cfg.read<QString>("annie")))
    {
        QString annieLatestReleasePageUrl = "https://github.com/iawia002/annie/releases/latest";
        QByteArray data;
        getData(annieLatestReleasePageUrl, data);
        reg.setPatternOptions(QRegularExpression::MultilineOption);
        QRegularExpressionMatch match = reg.match(data);
        if (match.hasMatch())
        {
            QString matched = match.captured(0);
            QString packageUrl = "https://github.com" + matched;
            getFile(packageUrl, appLocalDataPath + "/annie.zip");
            extractZIP(appLocalDataPath + "/annie.zip");
            QFile::remove(appLocalDataPath + "/annie.zip");
            cfg.write("annie", annie);
        }
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

    Config cfg;
    if (!QFile::exists(cfg.read<QString>("ykdl")))
        cfg.write("ykdl", "/usr/local/bin/ykdl");
    if (!QFile::exists(cfg.read<QString>("you-get")))
        cfg.write("you-get", "/usr/local/bin/you-get");
    if (!QFile::exists(cfg.read<QString>("youtube-dl")))
        cfg.write("youtube-dl", "/usr/local/bin/youtube-dl");
    if (!QFile::exists(cfg.read<QString>("annie")))
        cfg.write("annie", "/usr/local/bin/annie");
    if (!QFile::exists(cfg.read<QString>("ffmpeg")))
        cfg.write("ffmpeg", "/usr/local/bin/ffmpeg");
}

void DependenciesUpgrade::getFile(const QString &u, const QString &saveToFile)
{
    QByteArray data;
    getData(u, data);
    QFile f(saveToFile);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        f.write(data);
        f.close();
    }
}

void DependenciesUpgrade::getData(const QString &u,  QByteArray &data)
{
    QEventLoop loop;
    QNetworkRequest req(u);
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    QNetworkReply* reply = nam.get(req);
    NetworkReplyHelper helper(reply);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    reply->deleteLater();
    QObject::disconnect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    data = helper.content();
}

void DependenciesUpgrade::extractZIP(const QString &zip)
{
    // extract from zip package
    QZipReader cZip(zip);
    QFileInfo fi(zip);
    foreach(QZipReader::FileInfo item, cZip.fileInfoList())
    {
        if (item.isDir)
        {
            QDir d(fi.absolutePath() +"/"+ item.filePath);
            if (!d.exists())
                d.mkpath(fi.absolutePath() +"/"+ item.filePath);
        }

        if (item.isFile)
        {
            QFile file(fi.absolutePath() +"/"+ item.filePath);
            file.open(QIODevice::WriteOnly | QIODevice::Truncate);
            file.write(cZip.fileData(item.filePath));
            file.close();
        }
    }

    cZip.close();
}
