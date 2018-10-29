#include "browser.h"
#include "browserwindow.h"
#include "webview.h"
#include "linkresolver.h"
#include "waitingspinnerwidget.h"
#include "playdialog.h"
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

Browser::Browser(QObject* parent)
    : QObject(parent)
    , m_waitingSpinner(nullptr)
    , m_linkResolver(this)
    , m_nam(nullptr)
{
    connect(&m_linkResolver, &LinkResolver::resolvingFinished, this, &Browser::onResolved);
    connect(&m_linkResolver, &LinkResolver::resolvingError, this, &Browser::onResolvingError);
    connect(&m_playerProcess, &QProcess::errorOccurred, this, &Browser::onProcessError);
    connect(&m_playerProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Browser::onPlayerFinished);
    connect(&m_urlRequestInterceptor, &UrlRequestInterceptor::maybeMediaUrl, this, &Browser::onSniffedMediaUrl);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &Browser::onClipboardChanged);
}

void Browser::clearAtExit()
{
    stopWaiting();
    m_playerProcess.kill();
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

void Browser::doPlayByMediaPlayer(const QString &u, const QString &title)
{
    PlayDialog dlg(reinterpret_cast<QWidget*>(const_cast<BrowserWindow*>(mainWindow())));
    dlg.setMediaInfo(title, u);
    if (dlg.exec())
    {
        Tuple2 player = dlg.player();

        m_playerProcess.kill();
        waiting(false);
        QStringList args;
#if defined(Q_OS_MAC)
        QFileInfo fi(std::get<0>(player));
        if (fi.suffix() == "app")
        {
            m_playerProcess.setProgram("/usr/bin/open");
            args << std::get<0>(player) << "--args";
        }
#endif
        QString arg = std::get<1>(player);
        if (!arg.isEmpty())
            args << arg.split(" ");

        for (QString& a : args)
        {
            if (a == "{{referrer}}")
                a = "";
            else if (a == "{{title}}")
                a = title;
            else if (a == "{{site}}")
                a = title;
            else if (a == "{{user-agent}}")
                a = Config().read<QString>(QLatin1String("httpUserAgent"));
        }

        args << u;
        m_playerProcess.setArguments(args);

#if defined(Q_OS_MAC)
        if (fi.suffix() == "app")
        {
            m_playerProcess.setProgram("/usr/bin/open");
            return;
        }
#endif
        m_playerProcess.setProgram(std::get<0>(player));
        m_playerProcess.start();
        minimizeWindows();
    }
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
    mainWindow->showMaximized();
}

BrowserWindow *Browser::mainWindow()
{
    if (m_windows.isEmpty())
        newMainWindow();
    return m_windows[0];
}

BrowserWindow *Browser::newMainWindow()
{
    BrowserWindow* nmw = new BrowserWindow();
    addWindow(nmw);
    return nmw;
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

Websites &Browser::websites()
{
    return m_websites;
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

void Browser::doPlayByMediaPlayer(MediaInfoPtr mi)
{
    PlayDialog dlg(reinterpret_cast<QWidget*>(const_cast<BrowserWindow*>(mainWindow())));
    dlg.setMediaInfo(mi);
    if (dlg.exec())
    {
        Tuple2 player = dlg.player();
        StreamInfoPtr stream = dlg.media();

        m_playerProcess.kill();
        waiting(false);
        QStringList args;
#if defined(Q_OS_MAC)
        QFileInfo fi(std::get<0>(player));
        if (fi.suffix() == "app")
        {
            m_playerProcess.setProgram("/usr/bin/open");
            args << std::get<0>(player) << "--args";
        }
#endif
        QString arg = std::get<1>(player);
        if (!arg.isEmpty())
            args << arg.split(" ");

        for (QString& a : args)
        {
            if (a == "{{referrer}}")
                a = mi->url;
            else if (a == "{{title}}")
                a = mi->title;
            else if (a == "{{site}}")
                a = mi->site;
            else if (a == "{{user-agent}}")
                a = Config().read<QString>(QLatin1String("httpUserAgent"));
        }

        args << stream->urls;
        //TODO: too many urls cause large arguments, process may fail to start, generate a m3u8 instead
        m_playerProcess.setArguments(args);

#if defined(Q_OS_MAC)
        if (fi.suffix() == "app")
        {
            m_playerProcess.setProgram("/usr/bin/open");
            return;
        }
#endif
        m_playerProcess.setProgram(std::get<0>(player));
        m_playerProcess.start();
        minimizeWindows();
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

    if (mi->title.isEmpty() && mi->site.isEmpty())
    {
        QMessageBox::warning(mainWindow(),
                             tr("Error"), tr("Resolving link address failed! Please try again."), QMessageBox::Ok);
        return;
    }

    doPlayByMediaPlayer(mi);
}

void Browser::onResolvingError(const QString &u)
{
    stopWaiting();

    QMessageBox::warning(mainWindow(),
                         tr("Error"), tr("Resolving link address ") + u + tr(" failed!"), QMessageBox::Ok);
}

void Browser::onProcessError(QProcess::ProcessError error)
{
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
                         tr("Error on launching external player"), msg, QMessageBox::Ok);
}

void Browser::onPlayerFinished(int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/)
{
    stopWaiting();
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
}

void Browser::onSniffedMediaUrl(const QString &u)
{
    auto mw = const_cast<BrowserWindow*>(mainWindow());
    if (!mw->isCurrentVIPVideo())
        return;
    mw->recoverCurrentTabUrl();
    doPlayByMediaPlayer(u, mw->maybeVIPVideoTitle());
}

