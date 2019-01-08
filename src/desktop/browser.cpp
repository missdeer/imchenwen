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
    , m_playDialog(nullptr)
{
    connect(&m_linkResolver, &LinkResolver::done, this, &Browser::onNormalLinkResolved);
    connect(&m_linkResolver, &LinkResolver::error, this, &Browser::onNormalLinkResolvingError);
    connect(&m_vipResolver, &VIPResolver::done, this, &Browser::onVIPLinkResolved);
    connect(&m_vipResolver, &VIPResolver::error, this, &Browser::onVIPLinkResolvingError);
    connect(&m_sniffer, &Sniffer::done, this, &Browser::onSnifferDone);
    connect(&m_sniffer, &Sniffer::error, this, &Browser::onSnifferError);
    connect(&m_playerProcess, &QProcess::errorOccurred, this, &Browser::onProcessError);
    connect(&m_playerProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Browser::onPlayerFinished);
    connect(&m_mediaRelay, &MediaRelay::inputEnd, &m_httpHandler, &InMemoryHandler::onInputEnd);
    connect(&m_mediaRelay, &MediaRelay::newM3U8Ready, this, &Browser::onNewM3U8Ready);
    connect(&m_mediaRelay, &MediaRelay::transcodingFailed, this, &Browser::onTranscodingFailed);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &Browser::onClipboardChanged);
}

void Browser::clearAtExit()
{
    disconnect(&m_linkResolver, &LinkResolver::done, this, &Browser::onNormalLinkResolved);
    disconnect(&m_linkResolver, &LinkResolver::error, this, &Browser::onNormalLinkResolvingError);
    disconnect(&m_vipResolver, &VIPResolver::done, this, &Browser::onVIPLinkResolved);
    disconnect(&m_vipResolver, &VIPResolver::error, this, &Browser::onVIPLinkResolvingError);
    disconnect(&m_sniffer, &Sniffer::done, this, &Browser::onSnifferDone);
    disconnect(&m_sniffer, &Sniffer::error, this, &Browser::onSnifferError);
    disconnect(&m_playerProcess, &QProcess::errorOccurred, this, &Browser::onProcessError);
    disconnect(&m_playerProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Browser::onPlayerFinished);
    disconnect(&m_mediaRelay, &MediaRelay::inputEnd, &m_httpHandler, &InMemoryHandler::onInputEnd);
    disconnect(&m_mediaRelay, &MediaRelay::newM3U8Ready, this, &Browser::onNewM3U8Ready);
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
    resolveLink(u.trimmed());
}

void Browser::resolveVIPAndPlayByMediaPlayer(const QString &u)
{
    resolveVIPLink(u.trimmed());
}

void Browser::doPlay(PlayerPtr player, QStringList &videoUrls, const QString &audioUrl, const QString &subtitleUrl, const QString &title, const QString &referrer)
{
    m_playerProcess.kill();
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
    QString videoUrl = videoUrls[0];
    if (videoUrls.length() > 1)
    {
        videoUrl = m_mediaRelay.makeM3U8(player, videoUrls);
    }

    qDebug() << __FUNCTION__ << videoUrl;
    if (player->type() == Player::PT_DLNA)
    {
        if (QUrl(videoUrl).path().endsWith("m3u8", Qt::CaseInsensitive))
        {
            // DLNA not support m3u8, use ffmpeg to transcode to a single stream
            if (QUrl(videoUrl).hasQuery())
            {
                m_mediaRelay.setPlayer(player);
                m_mediaRelay.setTitle(title);
                // download the m3u8, extract the real stream urls, then regenerate m3u8 and invoke ffmpeg
                m_mediaRelay.processM3U8(videoUrl,referrer.toUtf8(),Config().read<QByteArray>(QLatin1String("httpUserAgent")));
                return;
            }
            videoUrl = m_mediaRelay.transcoding(videoUrl);
        }
        else if (QUrl(videoUrl).hasQuery())
        {
            // DLNA not support complex query string
            videoUrl = m_httpHandler.mapUrl(videoUrl);
        }
        qDebug() << __FUNCTION__ << "DLNA playing" << videoUrl ;
    }

    switch (player->type())
    {
    case Player::PT_BUILTIN:
        playByBuiltinPlayer(videoUrl, audioUrl, subtitleUrl, title, referrer);
        break;
    case Player::PT_DLNA:
        playByDLNARenderer(player, videoUrl, title, referrer);
        break;
    case Player::PT_EXTERNAL:
        playByExternalPlayer(player, videoUrl, audioUrl, subtitleUrl, title, referrer);
        break;
    }
    minimizeWindows();
}

void Browser::init()
{
    m_liveTVHelper.update();
    m_websites.update();
    m_vipResolver.update();
}

void Browser::play(const QString &originalUrl, MediaInfoPtr mi)
{
    if (m_playDialog)
    {
        m_playDialog->setMediaInfo(originalUrl, mi);
        return;
    }
    m_playDialog = new PlayDialog(reinterpret_cast<QWidget*>(const_cast<BrowserWindow*>(mainWindow())));
    m_playDialog->setMediaInfo(originalUrl, mi);
    int res = m_playDialog->exec();
    m_linkResolver.terminateResolvers();
    stopWaiting();
    if (res)
    {
        PlayerPtr player = m_playDialog->player();
        StreamInfoPtr video = m_playDialog->video();
        doPlay(player, video->urls, m_playDialog->audioUrl(), m_playDialog->subtitleUrl(), mi->title, mi->url);
    }
    delete m_playDialog;
    m_playDialog = nullptr;
}

void Browser::play(const QString& originalUrl, const QStringList &results, const QString &title)
{
    if (m_playDialog)
    {
        m_playDialog->setMediaInfo(originalUrl, title, results);
        return;
    }
    m_playDialog = new PlayDialog(reinterpret_cast<QWidget*>(const_cast<BrowserWindow*>(mainWindow())));
    m_playDialog->setMediaInfo(originalUrl, title, results);
    int res = m_playDialog->exec();
    m_vipResolver.stop();
    stopWaiting();
    if (res)
    {
        PlayerPtr player = m_playDialog->player();
        QString videoUrl = m_playDialog->videoUrl();
        doPlay(player, QStringList() << videoUrl, m_playDialog->audioUrl(), m_playDialog->subtitleUrl(), title, "");
    }
    delete m_playDialog;
    m_playDialog = nullptr;
}

void Browser::playByBuiltinPlayer(const QString &videoUrl, const QString &audioUrl, const QString &subtitle, const QString& title, const QString& referrer)
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
    m_builtinPlayer->subtitle(subtitle);
    m_builtinPlayer->playMedia(videoUrl, audioUrl);
}

void Browser::playByExternalPlayer(PlayerPtr player, const QString &videoUrl, const QString &audioUrl, const QString &subtitle, const QString &title, const QString &referrer)
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

    for (;;)
    {
        auto beginPos = std::find_if(args.begin(), args.end(), [](const QString& arg){ return arg.startsWith("\"");});
        auto endPos = std::find_if(beginPos, args.end(), [](const QString& arg){ return arg.endsWith("\"");});
        if (beginPos == args.end() || endPos == args.end())
        {
            break;
        }

        if (beginPos == endPos)
        {
            // an argument with single word wrapped by double quote
            QString var = *beginPos;
            *beginPos = var.mid(1, var.length() - 2); // trim double quote at begin & at end
            continue;
        }

        // an argument with several words wrapped by double quote
        QStringList vars;
        std::copy(beginPos, endPos+1, std::back_inserter(vars));
        QString var = vars.join(" ");
        *beginPos = var.mid(1, var.length() - 2); // trim double quote at begin & at end
        auto beginIndex = std::distance(args.begin(), beginPos);
        auto endIndex = std::distance(args.begin(), endPos);
        for(auto removeIndex = endIndex; removeIndex != beginIndex; --removeIndex)
            args.removeAt(static_cast<int>(removeIndex));
    }

    struct {
        const QString &variable;
        QString escape;
    } esc[] = {
        { referrer, "{{referrer}}"},
        { title, "{{title}}"},
        { title, "{{site}}"},
        { audioUrl, "{{audio}}"},
        { subtitle, "{{subtitle}}"},
        { Config().read<QString>(QLatin1String("httpUserAgent")), "{{user-agent}}"},
    };
    for (const auto& v : esc)
    {
        if (v.variable.isEmpty())
        {
            int index = args.indexOf(v.escape);
            args.removeAt(index);
            if (index > 0)
                args.removeAt(index -1);

            auto it = std::find_if(args.begin(), args.end(),
                                   [&](const QString& a){ return a.contains(v.escape);});
            if (args.end() != it)
                args.erase(it);
        }
    }

    for (QString& a : args)
    {
        a = a.replace("{{referrer}}", referrer);
        a = a.replace("{{title}}", title);
        a = a.replace("{{site}}", title);
        a = a.replace("{{audio}}", audioUrl);
        a = a.replace("{{subtitle}}", subtitle);
        a = a.replace("{{user-agent}}", Config().read<QString>(QLatin1String("httpUserAgent")));
    }

    args << videoUrl;
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
    if (m_playDialog)
        return;
    QClipboard *clipboard = QApplication::clipboard();
    QString originalText = clipboard->text().trimmed();
    if (!originalText.startsWith("https://") && !originalText.startsWith("http://"))
        return;
    if (!m_websites.isIn(QUrl(originalText)))
        return;
    if (m_websites.isIn(QUrl(originalText), "film"))
        m_sniffer.sniff(originalText);
    else
        m_linkResolver.resolve(originalText);
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

    if (u.startsWith("http://") || u.startsWith("https://"))
    {
        if (m_websites.isIn(QUrl(u), "china") || m_websites.isIn(QUrl(u), "abroad") )
            m_linkResolver.resolve(u);
        else
            m_sniffer.sniff(u);
    }
}

void Browser::resolveVIPLink(const QString &u)
{
    waiting();

    if (m_vipResolver.ready() && (u.startsWith("http://") || u.startsWith("https://")))
        m_vipResolver.resolve(u);
    else
    {
        QMessageBox::warning(mainWindow(),
                             tr("Error"),
                             tr("VIP resolver is not ready now, please try again later."),
                             QMessageBox::Ok);
    }
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

void Browser::onNormalLinkResolved(const QString& originalUrl, MediaInfoPtr results)
{
    Q_UNUSED(originalUrl);
    stopWaiting();

    if (results->ykdl.isEmpty() && results->you_get.isEmpty() && results->youtube_dl.isEmpty() && results->annie.isEmpty())
    {
        QMessageBox::warning(mainWindow(),
                             tr("Error"), tr("Resolving link address failed! Please try again."), QMessageBox::Ok);
        return;
    }

    play(originalUrl, results);
}

void Browser::onNormalLinkResolvingError(const QString &url, const QString &msg)
{
    stopWaiting();

    play(url, QStringList() << url, tr("%1 Play movie online directly\n%2").arg(msg).arg(url));
}

void Browser::onVIPLinkResolved(const QString& originalUrl, const QStringList &results)
{
    stopWaiting();

    auto mw = const_cast<BrowserWindow*>(mainWindow());
    play(originalUrl, results, mw->maybeVIPVideoTitle());
}

void Browser::onVIPLinkResolvingError()
{
    stopWaiting();

    QMessageBox::warning(mainWindow(),
                         tr("Error"),
                         tr("Resolving link address as VIP failed!"),
                         QMessageBox::Ok);
}

void Browser::onSnifferDone(const QString& originalUrl, const QString &result)
{
    stopWaiting();

    auto mw = const_cast<BrowserWindow*>(mainWindow());
    play(originalUrl, QStringList() << result, mw->maybeVIPVideoTitle());
}

void Browser::onSnifferError()
{
    stopWaiting();

    QMessageBox::warning(mainWindow(),
                         tr("Error"),
                         tr("Resolving link address failed!"),
                         QMessageBox::Ok);
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

void Browser::onNewM3U8Ready()
{
    QString media = m_mediaRelay.transcoding(QString("http://%1:51290/media.m3u8").arg(Util::getLocalAddress().toString()));
    qDebug() << __FUNCTION__ << "playing" << media ;
    playByDLNARenderer(m_mediaRelay.player(), media, m_mediaRelay.title(), "");
}

void Browser::onTranscodingFailed()
{
    QMessageBox::warning(mainWindow(),
                         tr("Error"),
                         tr("Transcoding failed."),
                         QMessageBox::Ok);
}

