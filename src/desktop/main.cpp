#include <QApplication>
#include <QMessageBox>
#include <QSplashScreen>
#include <QThreadPool>
#include <QTranslator>
#include <clocale>

#include "browser.h"
#include "browserwindow.h"

#if !defined(Q_OS_MAC)
#    include <QWebEngineSettings>
#    include <QtWebEngine>
#endif

QString getCommandLineUrlArgument()
{
    const QStringList args = QCoreApplication::arguments();
    if (args.count() > 1) {
        const QString lastArg = args.last();
        const bool isValidUrl = QUrl::fromUserInput(lastArg).isValid();
        if (isValidUrl)
            return lastArg;
    }
    return QString();
}

int main(int argc, char **argv)
{
#if !defined(Q_OS_WIN)
    // increase the number of file that can be opened.
    struct rlimit rl;
    getrlimit(RLIMIT_NOFILE, &rl);

    rl.rlim_cur = qMin((rlim_t)OPEN_MAX, rl.rlim_max);
    setrlimit(RLIMIT_NOFILE, &rl);
#endif
    QCoreApplication::setOrganizationName(QLatin1String("DForD Software"));
    QCoreApplication::setApplicationName(QObject::tr("imchenwen"));
    QCoreApplication::setApplicationVersion(QLatin1String("1.0"));
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

#if !defined(Q_OS_MAC)
    QtWebEngine::initialize();
#endif

    QApplication a(argc, argv);

    if (!QSslSocket::supportsSsl())
    {
        QMessageBox::critical(nullptr, QObject::tr("Critical error"), QObject::tr("SSL not supported, exit now."), QMessageBox::Ok);
        return -1;
    }

    QPixmap       pixmap(":/imchenwen256.png");
    QSplashScreen splash(pixmap);
    splash.show();
    a.processEvents();

    splash.showMessage("Detecting environment path...");

#if defined(Q_OS_WIN)
    auto pathEnv = qgetenv("PATH");
    pathEnv.append(";" % QDir::toNativeSeparators(QCoreApplication::applicationDirPath()));
    qputenv("PATH", pathEnv); // so that extensions can use main executable's Qt binaries
    qputenv("QT_PLUGIN_PATH", QDir::toNativeSeparators(QCoreApplication::applicationDirPath()).toUtf8());
#elif defined(Q_OS_MAC)
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("Frameworks");
    qputenv("DYLD_LIBRARY_PATH", dir.absolutePath().toUtf8());
    dir.cd("QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app/Contents/MacOS");
    qputenv("QT_MAC_DISABLE_FOREGROUND_APPLICATION_TRANSFORM", "1");
    QString qtWebEngineProcessPath = dir.absolutePath() + "/QtWebEngineProcess";
    if (QFile::exists(qtWebEngineProcessPath))
        qputenv("QTWEBENGINEPROCESS_PATH", qtWebEngineProcessPath.toUtf8());
#else
    auto pathEnv = qgetenv("LD_LIBRARY_PATH");
    pathEnv.append(":" % QCoreApplication::applicationDirPath());
    qputenv("LD_LIBRARY_PATH", pathEnv);
#endif

    a.processEvents();
    splash.showMessage("Loading translation file...");
    QString locale = QLocale::system().name();
    QTranslator translator;
    QTranslator qtTranslator;

    // main application and dynamic linked library locale
#if defined(Q_OS_MAC)
    QString localeDirPath = QApplication::applicationDirPath() + "/../Resources/translations";
#else
    QString localeDirPath = QApplication::applicationDirPath() + "/translations";
    if (!QDir(localeDirPath).exists())
    {
        localeDirPath = QApplication::applicationDirPath() + "/../translations";
    }
#endif

    if (!translator.load("imchenwen_" + locale, localeDirPath))
    {
        qDebug() << "loading " << locale << " from " << localeDirPath << " failed";
    }
    else
    {
        qDebug() << "loading " << locale << " from " << localeDirPath << " success";
        if (!a.installTranslator(&translator))
        {
            qDebug() << "installing translator failed ";
        }
    }

    // qt locale
    if (!qtTranslator.load("qt_" + locale, localeDirPath))
    {
        qDebug() << "loading " << locale << " from " << localeDirPath << " failed";
    }
    else
    {
        qDebug() << "loading " << locale << " from " << localeDirPath << " success";
        if (!a.installTranslator(&qtTranslator))
        {
            qDebug() << "installing qt translator failed ";
        }
    }

    // Qt sets the locale in the QApplication constructor, but libmpv requires
    // the LC_NUMERIC category to be set to "C", so change it back.
    std::setlocale(LC_NUMERIC, "C");

#if defined(Q_OS_MAC)
    QApplication::setWindowIcon(QIcon(":imchenwen.icns"));
#else
    QApplication::setWindowIcon(QIcon(":imchenwen.ico"));
#endif

    a.processEvents();
    splash.showMessage("Loading settings...");
    Browser& browser = Browser::instance();
    browser.loadSettings();

    const QString url = getCommandLineUrlArgument();
    a.processEvents();
    splash.showMessage("Creating main window...");
    BrowserWindow *window = browser.mainWindow();

    a.processEvents();
    splash.showMessage("Initializing application...");
    if (!url.isEmpty())
    {
        browser.init(true);
        a.processEvents();
        splash.showMessage("Resolving video link...");
        browser.resolveUrl(url);
    }
    else
    {
        browser.init(false);
        a.processEvents();
        splash.showMessage("Loading home page...");
        window->loadHomePage();
    }

    splash.finish(window);
    return QApplication::exec();
}
