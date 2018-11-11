#include "browser.h"
#include "browserwindow.h"
#include "webview.h"
#include "linkresolver.h"
#include "waitingspinnerwidget.h"
#include "playdialog.h"
#include "playerview.h"
#include "dlnaplayerview.h"
#include "util.h"
#include <QAuthenticator>
#include <QMessageBox>
#include <QFileInfo>
#include <QFile>
#include <QApplication>
#include <QClipboard>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QNetworkProxy>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QThread>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QDesktopWidget>

static void setUserStyleSheet(QWebEngineProfile *profile, const QString &styleSheet, BrowserWindow *mainWindow = nullptr)
{
    Q_ASSERT(profile);
    QString scriptName(QStringLiteral("userStyleSheet"));
    QWebEngineScript script;
    QList<QWebEngineScript> styleSheets = profile->scripts()->findScripts(scriptName);
    if (!styleSheets.isEmpty())
        script = styleSheets.first();
    Q_FOREACH (const QWebEngineScript &s, styleSheets)
        profile->scripts()->remove(s);

    if (script.isNull()) {
        script.setName(scriptName);
        script.setInjectionPoint(QWebEngineScript::DocumentReady);
        script.setRunsOnSubFrames(true);
        script.setWorldId(QWebEngineScript::ApplicationWorld);
    }
    QString source = QString::fromLatin1("(function() {"\
                                         "var css = document.getElementById(\"_qt_testBrowser_userStyleSheet\");"\
                                         "if (css == undefined) {"\
                                         "    css = document.createElement(\"style\");"\
                                         "    css.type = \"text/css\";"\
                                         "    css.id = \"_qt_testBrowser_userStyleSheet\";"\
                                         "    document.head.appendChild(css);"\
                                         "}"\
                                         "css.innerText = \"%1\";"\
                                         "})()").arg(styleSheet);
    script.setSourceCode(source);
    profile->scripts()->insert(script);
    // run the script on the already loaded views
    // this has to be deferred as it could mess with the storage initialization on startup
    if (mainWindow)
        QMetaObject::invokeMethod(mainWindow, "runScriptOnOpenViews", Qt::QueuedConnection, Q_ARG(QString, source));
}

Browser::Browser(QObject *parent)
    : QObject(parent)
    , m_waitingSpinner(nullptr)
    , m_linkResolver(this)
    , m_nam(nullptr)
    , m_builtinPlayer(nullptr)
    , m_liveTVHelper("liveTV", "liveTVSubscription")
    , m_vipVideoHelper("vipVideo", "vipVideoSubscription")
{
    connect(&m_linkResolver, &LinkResolver::resolvingFinished, this, &Browser::onResolved);
    connect(&m_linkResolver, &LinkResolver::resolvingError, this, &Browser::onResolvingError);
    connect(&m_playerProcess, &QProcess::errorOccurred, this, &Browser::onProcessError);
    connect(&m_playerProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Browser::onPlayerFinished);
    connect(&m_ffmpegProcess, &QProcess::errorOccurred, [this](){
        emit m_httpHandler.inputEnd();
    });
    connect(&m_ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this](){
        emit m_httpHandler.inputEnd();
    });
    connect(&m_urlRequestInterceptor, &UrlRequestInterceptor::maybeMediaUrl, this, &Browser::onSniffedMediaUrl);
    //connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &Browser::onClipboardChanged);
    m_liveTVHelper.update();
    m_vipVideoHelper.update();
}

void Browser::clearAtExit()
{
    stopWaiting();
    m_playerProcess.kill();
    if (m_httpServer.isListening())
        m_httpServer.close();
}

Browser::~Browser()
{
}

Browser &Browser::instance()
{
    static Browser browser;
    return browser;
}

void Browser::loadSettings()
{
    Config cfg;
    QWebEngineSettings *defaultSettings = QWebEngineSettings::globalSettings();
    QWebEngineProfile *defaultProfile = QWebEngineProfile::defaultProfile();

    QString standardFontFamily = defaultSettings->fontFamily(QWebEngineSettings::StandardFont);
    int standardFontSize = defaultSettings->fontSize(QWebEngineSettings::DefaultFontSize);
    QFont standardFont = QFont(standardFontFamily, standardFontSize);
    standardFont = qvariant_cast<QFont>(cfg.read(QLatin1String("standardFont"), QVariant(standardFont)));
    defaultSettings->setFontFamily(QWebEngineSettings::StandardFont, standardFont.family());
    defaultSettings->setFontSize(QWebEngineSettings::DefaultFontSize, standardFont.pointSize());

    QString fixedFontFamily = defaultSettings->fontFamily(QWebEngineSettings::FixedFont);
    int fixedFontSize = defaultSettings->fontSize(QWebEngineSettings::DefaultFixedFontSize);
    QFont fixedFont = QFont(fixedFontFamily, fixedFontSize);
    fixedFont = qvariant_cast<QFont>(cfg.read(QLatin1String("fixedFont"),QVariant(fixedFont)));
    defaultSettings->setFontFamily(QWebEngineSettings::FixedFont, fixedFont.family());
    defaultSettings->setFontSize(QWebEngineSettings::DefaultFixedFontSize, fixedFont.pointSize());

    defaultSettings->setAttribute(QWebEngineSettings::JavascriptEnabled, cfg.read<bool>(QLatin1String("enableJavascript"), true));
    defaultSettings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, cfg.read<bool>(QLatin1String("enableScrollAnimator"), true));

    defaultSettings->setAttribute(QWebEngineSettings::PluginsEnabled, cfg.read<bool>(QLatin1String("enablePlugins"), true));

    defaultSettings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);

    QString css = cfg.read<QString>(QLatin1String("userStyleSheet"));
    setUserStyleSheet(defaultProfile, css, mainWindow());

    defaultProfile->setHttpUserAgent(cfg.read<QString>(QLatin1String("httpUserAgent")));
    defaultProfile->setHttpAcceptLanguage(cfg.read<QString>(QLatin1String("httpAcceptLanguage")));

    switch (cfg.read<int>(QLatin1String("faviconDownloadMode"), 1)) {
    case 0:
        defaultSettings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
        break;
    case 1:
        defaultSettings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, true);
        defaultSettings->setAttribute(QWebEngineSettings::TouchIconsEnabled, false);
        break;
    case 2:
        defaultSettings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, true);
        defaultSettings->setAttribute(QWebEngineSettings::TouchIconsEnabled, true);
        break;
    }

    QWebEngineProfile::PersistentCookiesPolicy persistentCookiesPolicy = QWebEngineProfile::PersistentCookiesPolicy(cfg.read<int>(QLatin1String("persistentCookiesPolicy")));
    defaultProfile->setPersistentCookiesPolicy(persistentCookiesPolicy);
    QString pdataPath = cfg.read<QString>(QLatin1String("persistentDataPath"));
    defaultProfile->setPersistentStoragePath(pdataPath);

    m_urlRequestInterceptor.updateVIPVideos();
    defaultProfile->setRequestInterceptor(&m_urlRequestInterceptor);

    QNetworkProxy proxy;
    if (cfg.read<bool>(QLatin1String("enableProxy"), false)) {
        if (cfg.read<int>(QLatin1String("proxyType"), 0) == 0)
            proxy = QNetworkProxy::Socks5Proxy;
        else
            proxy = QNetworkProxy::HttpProxy;
        proxy.setHostName(cfg.read<QString>(QLatin1String("proxyHostName")));
        proxy.setPort(cfg.read<quint16>(QLatin1String("proxyPort"), 1080));
        proxy.setUser(cfg.read<QString>(QLatin1String("proxyUserName")));
        proxy.setPassword(cfg.read<QString>(QLatin1String("proxyPassword")));
    }
    QNetworkProxy::setApplicationProxy(proxy);
}

void Browser::resolveAndPlayByMediaPlayer(const QString &u)
{
    resolveLink(u);
}

void Browser::doPlay(PlayerPtr player, QStringList& urls, const QString& title, const QString& referrer)
{
    m_playerProcess.kill();
    waiting(false);
    if (!m_httpServer.isListening())
    {
        m_httpServer.listen(QHostAddress::Any, 51290);
        m_httpServer.setHandler(&m_httpHandler);
    }

    QString media = urls[0];
    QString ext = QFileInfo(QUrl(media).path()).suffix();
    if (urls.length() > 1)
    {
        // make a m3u8
        m_httpHandler.clear();
        int duration = 1500 / urls.length();
        QByteArray m3u8;
        m3u8.append(QString("#EXTM3U\n#EXT-X-TARGETDURATION:%1\n").arg(duration > 8 ? duration + 3 : 8).toUtf8());
        if (player->type() == Player::PT_DLNA)
        {
            for (const auto & u : urls)
            {
                m3u8.append(QString("#EXTINF:%1,\n%2\n")
                            .arg(duration > 5 ? duration : 5)
                            .arg(m_httpHandler.mapUrl(u))
                            .toUtf8());
            }
        }
        else
        {
            for (const auto & u : urls)
            {
                m3u8.append(QString("#EXTINF:%1,\n%2\n")
                            .arg(duration > 5 ? duration : 5)
                            .arg(u)
                            .toUtf8());
            }
        }
        m3u8.append("#EXT-X-ENDLIST\n");
        m_httpHandler.setM3U8(m3u8);
        if (!referrer.isEmpty())
        {
            m_httpHandler.setReferrer(referrer.toUtf8());
            m_httpHandler.setUserAgent(Config().read<QByteArray>(QLatin1String("httpUserAgent")));
        }
        media = QString("http://%1:51290/media.m3u8").arg(Util::getLocalAddress().toString());
    }

    qDebug() << media;
    if (player->type() == Player::PT_DLNA
            && QUrl(media).path().endsWith("media.m3u8", Qt::CaseInsensitive))
    {
        // DLNA not support m3u8, use ffmpeg to transcode to a single stream

        // ffmpeg.exe -y -protocol_whitelist "file,http,https,tcp,tls"  -i test.m3u8 -c:v libx265 -an -x265-params crf=25 -f mpegts udp://127.0.0.1:12345
        m_ffmpegProcess.kill();

        m_ffmpegProcess.setProgram(Config().read<QString>("ffmpeg"));
        if (ext.compare("265ts", Qt::CaseInsensitive) == 0)
        {
            m_ffmpegProcess.setArguments(QStringList() << "-y"
                                         << "-protocol_whitelist" << "file,http,https,tcp,tls"
                                         << "-i" << media
                                         << "-c:v" << "libx265"
                                         << "-c:a" << "aac"
                                         << "-copyts"
                                         << QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/media.ts"));
            // serve http://...:51290/media.ext
            media = QString("http://%1:51290/media.ts").arg(Util::getLocalAddress().toString());
        }
        else
        {
            m_ffmpegProcess.setArguments(QStringList() << "-y"
                                         << "-protocol_whitelist" << "file,http,https,tcp,tls"
                                         << "-i" << media
                                         << "-c:v" << "copy"
                                         << "-c:a" << "aac"
                                         << "-copyts"
                                         //<< "-bsf:a" << "aac_adtstoasc" // need for flv output
                                         << QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/media.ts"));

            // serve http://...:51290/media.ext
            media = QString("http://%1:51290/media.ts").arg(Util::getLocalAddress().toString());
        }
        m_ffmpegProcess.start();
    }

    qDebug() << "playing" << media ;
    switch (player->type())
    {
    case Player::PT_BUILTIN:
        playByBuiltinPlayer(media, title, referrer);
        break;
    case Player::PT_DLNA:
        playByDLNARenderer(player, media, title, referrer);
        break;
    case Player::PT_EXTERNAL:
        playByExternalPlayer(player, media, title, referrer);
        break;
    }
    minimizeWindows();
}

void Browser::play(const QString &u, const QString &title)
{
    PlayDialog dlg(reinterpret_cast<QWidget*>(const_cast<BrowserWindow*>(mainWindow())));
    dlg.setMediaInfo(title, u);
    if (dlg.exec())
    {
        PlayerPtr player = dlg.player();
        doPlay(player, QStringList() << u, title, "");
    }
}

void Browser::play(MediaInfoPtr mi)
{
    PlayDialog dlg(reinterpret_cast<QWidget*>(const_cast<BrowserWindow*>(mainWindow())));
    dlg.setMediaInfo(mi);
    if (dlg.exec())
    {
        PlayerPtr player = dlg.player();
        StreamInfoPtr stream = dlg.media();
        doPlay(player, stream->urls, mi->title, mi->url);
    }
}

void Browser::playByBuiltinPlayer(const QString &url, const QString& title, const QString& referrer)
{
    if (!m_builtinPlayer)
    {
        m_builtinPlayer = new PlayerView();
        connect(m_builtinPlayer, &PlayerView::finished, this, &Browser::onPlayerFinished);
    }

    m_builtinPlayer->show();
    m_builtinPlayer->title(title);
    m_builtinPlayer->referrer(referrer);
    m_builtinPlayer->userAgent(Config().read<QString>(QLatin1String("httpUserAgent")));
    m_builtinPlayer->playMedia(url);
}

void Browser::playByExternalPlayer(PlayerPtr player, const QString &url, const QString &title, const QString &referrer)
{
    QStringList args;
#if defined(Q_OS_MAC)
    QFileInfo fi(player->name());
    if (fi.suffix() == "app")
    {
        m_playerProcess.setProgram("/usr/bin/open");
        args << player->name() << "--args";
    }
#endif
    QString arg = player->arguments();
    if (!arg.isEmpty())
        args << arg.split(" ");

    for (QString& a : args)
    {
        a = a.replace("{{referrer}}", referrer);
        a = a.replace("{{title}}", title);
        a = a.replace("{{site}}", title);
        a = a.replace("{{user-agent}}", Config().read<QString>(QLatin1String("httpUserAgent")));
    }

    args << url;
    m_playerProcess.setArguments(args);

#if defined(Q_OS_MAC)
    if (fi.suffix() == "app")
    {
        m_playerProcess.setProgram("/usr/bin/open");
        return;
    }
#endif
    m_playerProcess.setProgram(player->name());
    m_playerProcess.start();
}

void Browser::playByDLNARenderer(PlayerPtr player, const QString &url, const QString &title, const QString &referrer)
{
    if (url.isEmpty())
        return;

    if (!m_dlnaPlayer)
    {
        m_dlnaPlayer = new DLNAPlayerView();
        connect(m_dlnaPlayer, &DLNAPlayerView::finished, this, &Browser::onPlayerFinished);
    }

    m_dlnaPlayer->title(title);
    m_dlnaPlayer->referrer(referrer);
    m_dlnaPlayer->userAgent(Config().read<QString>(QLatin1String("httpUserAgent")));
    m_dlnaPlayer->setRenderer(player->name());
    m_dlnaPlayer->playMedia(url);
    m_dlnaPlayer->show();
}

void Browser::onClipboardChanged()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString originalText = clipboard->text();

    if (originalText.startsWith("http://") || originalText.startsWith("https://"))
    {
        m_linkResolver.resolve(originalText);
    }
}

QVector<BrowserWindow*> Browser::windows()
{
    return m_windows;
}

void Browser::addWindow(BrowserWindow *mainWindow)
{
    if (m_windows.contains(mainWindow))
        return;
    m_windows.prepend(mainWindow);
    QObject::connect(mainWindow, &QObject::destroyed, [this, mainWindow]() {
        m_windows.removeOne(mainWindow);
        if (m_windows.isEmpty())
            clearAtExit();
    });
    mainWindow->show();
    mainWindow->center();
}

BrowserWindow *Browser::mainWindow()
{
    if (m_windows.isEmpty())
        newMainWindow();
    return m_windows[0];
}

BrowserWindow *Browser::newMainWindow()
{
    BrowserWindow *nmw = new BrowserWindow();
    addWindow(nmw);
    return nmw;
}

Kast &Browser::kast()
{
    return m_kast;
}

void Browser::waiting(bool disableParent /*= true*/)
{
    if (!m_waitingSpinner)
    {
        m_waitingSpinner = new WaitingSpinnerWidget(mainWindow(), true, disableParent);

        m_waitingSpinner->setRoundness(70.0);
        m_waitingSpinner->setMinimumTrailOpacity(15.0);
        m_waitingSpinner->setTrailFadePercentage(30.0);
        m_waitingSpinner->setNumberOfLines(12);
        m_waitingSpinner->setLineLength(40);
        m_waitingSpinner->setLineWidth(5);
        m_waitingSpinner->setInnerRadius(10);
        m_waitingSpinner->setRevolutionsPerSecond(1);
        //        m_waitingSpinner->setColor(QColor(81, 4, 71));
    }
    if (m_waitingSpinner->isSpinning())
        m_waitingSpinner->stop();

    m_waitingSpinner->start();
}

void Browser::resolveLink(const QString &u)
{
    waiting();

    m_linkResolver.resolve(u);
}

void Browser::minimizeWindows()
{
    for ( auto w : m_windows)
    {
        if (w->isMaximized())
        {
            m_windowsState[w] = isMaximized;
            w->showMinimized();
        }
        else if (w->isMinimized())
        {
            m_windowsState[w] = isMinimized;
        }
        else if (!w->isVisible())
        {
            m_windowsState[w] = isHidden;
        }
        else
        {
            m_windowsState[w] = isNormal;
            w->showMinimized();
        }
    }
}

void Browser::clean()
{
    qDeleteAll(m_windows);
    m_windows.clear();
}

void Browser::stopWaiting()
{
    if (m_waitingSpinner)
    {
        if (m_waitingSpinner->isSpinning())
            m_waitingSpinner->stop();
        delete m_waitingSpinner;
        m_waitingSpinner = nullptr;
    }
}

void Browser::onResolved(MediaInfoPtr mi)
{
    stopWaiting();

    if (mi->ykdl.isEmpty() && mi->you_get.isEmpty() && mi->youtube_dl.isEmpty() && mi->annie.isEmpty())
    {
        QMessageBox::warning(mainWindow(),
                             tr("Error"), tr("Resolving link address failed! Please try again."), QMessageBox::Ok);
        return;
    }

    play(mi);
}

void Browser::onResolvingError(const QString &u)
{
    stopWaiting();

    play(u, tr("Play movie online directly\n%1").arg(u));
}

void Browser::onProcessError(QProcess::ProcessError error)
{
    onPlayerFinished(1, QProcess::CrashExit);
    QString msg;
    switch(error)
    {
    case QProcess::FailedToStart:
        msg = tr("The process failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program.");
        break;
    case QProcess::Crashed:
        msg = tr("The process crashed some time after starting successfully.");
        break;
    default:
        msg = tr("An unknown error occurred.");
        break;
    }
    QMessageBox::warning(mainWindow(),
                         tr("Launching external player failed, please try built-in player"), msg, QMessageBox::Ok);
}

void Browser::onPlayerFinished(int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/)
{
    stopWaiting();
    m_ffmpegProcess.kill();
    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/media.ts");
    auto mw = const_cast<BrowserWindow*>(mainWindow());
    mw->currentVIPVideoGoBack();
    for (auto w : m_windows)
    {
        switch (m_windowsState[w])
        {
        case isMaximized:
            w->showMaximized();
            break;
        case isNormal:
            w->showNormal();
            break;
        default:
            break;
        }
    }
    m_windowsState.clear();
    if (m_builtinPlayer)
    {
        m_builtinPlayer->deleteLater();
        m_builtinPlayer = nullptr;
    }
    if (m_dlnaPlayer)
    {
        m_dlnaPlayer->deleteLater();
        m_dlnaPlayer = nullptr;
    }
}

void Browser::onSniffedMediaUrl(const QString &u)
{
    auto mw = const_cast<BrowserWindow*>(mainWindow());
    if (!mw->isCurrentVIPVideo())
        return;
    mw->recoverCurrentTabUrl();
    play(u, mw->maybeVIPVideoTitle());
}

