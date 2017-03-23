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

static void setUserStyleSheet(QWebEngineProfile *profile, const QString &styleSheet, BrowserWindow *mainWindow = 0)
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
{
    connect(&m_linkResolver, &LinkResolver::resolvingFinished, this, &Browser::resolvingFinished);
    connect(&m_linkResolver, &LinkResolver::resolvingError, this, &Browser::resolvingError);
    connect(&m_linkResolver, &LinkResolver::resolvingSilentFinished, this, &Browser::resolvingSilentFinished);
    connect(&m_linkResolver, &LinkResolver::resolvingSilentError, this, &Browser::resolvingSilentError);
    connect(&m_process, &QProcess::errorOccurred, this, &Browser::errorOccurred);
    Config cfg;
    if (cfg.read<bool>("inChinaLocalMode") || cfg.read<bool>("abroadLocalMode"))
    {
        startParsedProcess();
    }

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &Browser::clipboardChanged);
}

Browser::~Browser()
{
    clean();

    if (m_waitingSpinner)
    {
        if (m_waitingSpinner->isSpinning())
            m_waitingSpinner->stop();
        delete m_waitingSpinner;
    }
}

Browser &Browser::instance()
{
    static Browser browser;
    return browser;
}

void Browser::loadSettings()
{
    Config cfg;
    cfg.beginGroup(QLatin1String("websettings"));

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

    cfg.endGroup();
    cfg.beginGroup(QLatin1String("cookies"));

    QWebEngineProfile::PersistentCookiesPolicy persistentCookiesPolicy = QWebEngineProfile::PersistentCookiesPolicy(cfg.read<int>(QLatin1String("persistentCookiesPolicy")));
    defaultProfile->setPersistentCookiesPolicy(persistentCookiesPolicy);
    QString pdataPath = cfg.read<QString>(QLatin1String("persistentDataPath"));
    defaultProfile->setPersistentStoragePath(pdataPath);

    cfg.endGroup();

    cfg.beginGroup(QLatin1String("proxy"));
    QNetworkProxy proxy;
    if (cfg.read<bool>(QLatin1String("enabled"), false)) {
        if (cfg.read<int>(QLatin1String("type"), 0) == 0)
            proxy = QNetworkProxy::Socks5Proxy;
        else
            proxy = QNetworkProxy::HttpProxy;
        proxy.setHostName(cfg.read<QString>(QLatin1String("hostName")));
        proxy.setPort(cfg.read<int>(QLatin1String("port"), 1080));
        proxy.setUser(cfg.read<QString>(QLatin1String("userName")));
        proxy.setPassword(cfg.read<QString>(QLatin1String("password")));
    }
    QNetworkProxy::setApplicationProxy(proxy);
    cfg.endGroup();
}

void Browser::playByMediaPlayer(const QUrl& u)
{
    resolveLink(u);
}

void Browser::playVIPByMediaPlayer(const QUrl &u)
{

}

void Browser::clipboardChanged()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString originalText = clipboard->text();

    if (originalText.startsWith("http://") || originalText.startsWith("https://"))
    {
        m_linkResolver.resolve(QUrl(originalText), true);
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
    });
    mainWindow->show();
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

void Browser::resolveLink(const QUrl &u)
{
    if (!m_waitingSpinner)
    {
        m_waitingSpinner = new WaitingSpinnerWidget(mainWindow());

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

    m_linkResolver.resolve(u);
}

void Browser::doPlayByMediaPlayer(MediaInfoPtr mi)
{
    PlayDialog dlg(reinterpret_cast<QWidget*>(const_cast<BrowserWindow*>(mainWindow())));
    dlg.setMediaInfo(mi);
    if (dlg.exec())
    {
        Tuple2 player = dlg.player();
        StreamInfoPtr stream = dlg.media();

        if (m_process.state() != QProcess::NotRunning)
        {
            m_process.terminate();
        }

        QStringList args;
        m_process.setProgram(std::get<0>(player));
#if defined(Q_OS_MAC)
        QFileInfo fi(std::get<0>(player));
        if (fi.suffix() == "app")
        {
            m_process.setProgram("/usr/bin/open");
            args << std::get<0>(player) << "--args";
        }
#endif
        QString arg = std::get<1>(player);
        if (!arg.isEmpty())
            args << arg.split(" ");
        args << stream->urls;

#if defined(Q_OS_MAC)
        if (fi.suffix() == "app")
        {
            m_process.start("/usr/bin/open", args);
            return;
        }
#endif
        m_process.start(std::get<0>(player), args);
    }
}

void Browser::clean()
{
    qDeleteAll(m_windows);
    m_windows.clear();
}

void Browser::resolvingFinished(MediaInfoPtr mi)
{
    if (m_waitingSpinner->isSpinning())
        m_waitingSpinner->stop();
    m_waitingSpinner->deleteLater();
    m_waitingSpinner = nullptr;

    if (mi->title.isEmpty() && mi->site.isEmpty())
    {
        QMessageBox::warning(mainWindow(),
                             tr("Error"), tr("Resolving link address failed! Please try again."), QMessageBox::Ok);
        return;
    }

    doPlayByMediaPlayer(mi);
}

void Browser::resolvingError(const QUrl& u)
{
    if (m_waitingSpinner->isSpinning())
        m_waitingSpinner->stop();
    m_waitingSpinner->deleteLater();
    m_waitingSpinner = nullptr;

    QMessageBox::warning(mainWindow(),
                         tr("Error"), tr("Resolving link address ") + u.toString() + tr(" failed!"), QMessageBox::Ok);
}

void Browser::resolvingSilentFinished(MediaInfoPtr mi)
{
    if (mi->title.isEmpty() && mi->site.isEmpty())
    {
        return;
    }

    if (!m_windows.isEmpty())
    {
        m_windows.at(0)->activateWindow();
        m_windows.at(0)->raise();
    }
    doPlayByMediaPlayer(mi);
}

void Browser::resolvingSilentError(const QUrl&)
{

}

void Browser::errorOccurred(QProcess::ProcessError error)
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

void Browser::startParsedProcess()
{
    QString parsedPath = QApplication::applicationDirPath();
#if defined(Q_OS_WIN)
    parsedPath.append("/parsed.exe");
#else
    parsedPath.append("/../Resources/parsed");
#endif
    if (!QFile::exists(parsedPath))
    {
        QMessageBox::information(nullptr, tr("Notice"), tr("Can't launch parsed process for link resolving, please launch it by yourself."), QMessageBox::Ok);
    }
    else
    {
        if (m_parsedProcess.state() == QProcess::Running)
            m_parsedProcess.terminate();

        m_parsedProcess.start(parsedPath, QStringList() << "-l");
    }
}

void Browser::stopParsedProcess()
{
    m_parsedProcess.terminate();
}

void Browser::changeParsedProcessState()
{
    Config cfg;
    if (cfg.read<bool>("inChinaLocalMode") || cfg.read<bool>("abroadLocalMode"))
    {
        Browser::instance().startParsedProcess();
    }
    else
    {
        Browser::instance().stopParsedProcess();
    }
}
