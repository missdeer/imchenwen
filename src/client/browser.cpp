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
    , m_builtinPlayer(nullptr)
    , m_liveTVHelper("liveTV", "liveTVSubscription")
    , m_vipVideoHelper("vipVideo", "vipVideoSubscription")
    , m_playDialog(nullptr)
{
    connect(&m_linkResolver, &LinkResolver::resolvingFinished, this, &Browser::onResolved);
    connect(&m_linkResolver, &LinkResolver::resolvingError, this, &Browser::onResolvingError);
    connect(&m_playerProcess, &QProcess::errorOccurred, this, &Browser::onProcessError);
    connect(&m_playerProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Browser::onPlayerFinished);
    connect(&m_mediaRelay, &MediaRelay::inputEnd, &m_httpHandler, &InMemoryHandler::inputEnd);
    connect(&m_mediaRelay, &MediaRelay::newM3U8Ready, this, &Browser::onNewM3U8Ready);
    connect(&m_urlRequestInterceptor, &UrlRequestInterceptor::maybeMediaUrl, this, &Browser::onSniffedMediaUrl);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &Browser::onClipboardChanged);
}

void Browser::clearAtExit()
{
    disconnect(&m_linkResolver, &LinkResolver::resolvingFinished, this, &Browser::onResolved);
    disconnect(&m_linkResolver, &LinkResolver::resolvingError, this, &Browser::onResolvingError);
    disconnect(&m_playerProcess, &QProcess::errorOccurred, this, &Browser::onProcessError);
    disconnect(&m_playerProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Browser::onPlayerFinished);
    disconnect(&m_mediaRelay, &MediaRelay::inputEnd, &m_httpHandler, &InMemoryHandler::inputEnd);
    disconnect(&m_mediaRelay, &MediaRelay::newM3U8Ready, this, &Browser::onNewM3U8Ready);
    disconnect(&m_urlRequestInterceptor, &UrlRequestInterceptor::maybeMediaUrl, this, &Browser::onSniffedMediaUrl);
    disconnect(QApplication::clipboard(), &QClipboard::dataChanged, this, &Browser::onClipboardChanged);
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
    m_httpHandler.clear();
    if (!m_httpServer.isListening())
    {
        m_httpServer.listen(QHostAddress::Any, 51290);
        m_httpServer.setHandler(&m_httpHandler);
    }
    if (!referrer.isEmpty())
    {
        m_httpHandler.setReferrer(referrer.toUtf8());
        m_httpHandler.setUserAgent(Config().read<QByteArray>(QLatin1String("httpUserAgent")));
    }
    QString media = urls[0];
    if (urls.length() > 1)
    {
        media = m_mediaRelay.makeM3U8(player, urls);
    }

    qDebug() << __FUNCTION__ << media;
    if (player->type() == Player::PT_DLNA)
    {
        if (QUrl(media).path().endsWith("m3u8", Qt::CaseInsensitive))
        {
            // DLNA not support m3u8, use ffmpeg to transcode to a single stream
            if (QUrl(media).hasQuery())
            {
                m_mediaRelay.setPlayer(player);
                m_mediaRelay.setTitle(title);
                // download the m3u8, extract the real stream urls, then regenerate m3u8 and invoke ffmpeg
                m_mediaRelay.processM3U8(media,referrer.toUtf8(),Config().read<QByteArray>(QLatin1String("httpUserAgent")));
                return;
            }
            media = m_mediaRelay.transcoding(media);
        }
        else if (QUrl(media).hasQuery())
        {
            // DLNA not support complex query string
            media = m_httpHandler.mapUrl(media);
        }
        qDebug() << __FUNCTION__ << "DLNA playing" << media ;
    }

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
    if (m_playDialog)
    {
        m_playDialog->setMediaInfo(title, u);
        return;
    }
    m_playDialog = new PlayDialog(reinterpret_cast<QWidget*>(const_cast<BrowserWindow*>(mainWindow())));
    m_playDialog->setMediaInfo(title, u);
    if (m_playDialog->exec())
    {
        PlayerPtr player = m_playDialog->player();
        doPlay(player, QStringList() << u, title, "");
    }
    else
    {
        playerStopped();
    }
    delete m_playDialog;
    m_playDialog = nullptr;
}

void Browser::init()
{
    m_liveTVHelper.update();
    m_vipVideoHelper.update();
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
    if (originalText.startsWith("https://") || originalText.startsWith("http://"))
    {
        if (m_websites.isIn(QUrl(originalText)))
        {
            m_linkResolver.resolve(originalText);
        }
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

Websites &Browser::shortcuts()
{
    return m_websites;
}

QNetworkAccessManager &Browser::networkAccessManager()
{
    return m_nam;
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
    playerStopped();
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

void Browser::playerStopped()
{
    stopWaiting();
    m_httpHandler.clear();
    m_mediaRelay.stop();
    auto mw = const_cast<BrowserWindow*>(mainWindow());
    mw->resetCurrentView();
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

void Browser::onPlayerFinished(int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/)
{
    playerStopped();
}

void Browser::onSniffedMediaUrl(const QString &u)
{
    auto mw = const_cast<BrowserWindow*>(mainWindow());
    auto wv = mw->currentTab();
    auto url = wv->url();
    if (m_websites.isIn(url))
    {
        qDebug() << __FUNCTION__ << "shortcut url, ignore";
        return;
    }
    mw->recoverCurrentTabUrl();
    play(u, mw->maybeVIPVideoTitle());
}

void Browser::onNewM3U8Ready()
{
    QString media = m_mediaRelay.transcoding(QString("http://%1:51290/media.m3u8").arg(Util::getLocalAddress().toString()));
    qDebug() << __FUNCTION__ << "playing" << media ;
    playByDLNARenderer(m_mediaRelay.player(), media, m_mediaRelay.title(), "");
}

