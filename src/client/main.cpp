#include "browser.h"
#include "browserwindow.h"
#include <QApplication>
#include <QtWebEngine>
#include <QWebEngineSettings>
#include <QTranslator>

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
    QCoreApplication::setOrganizationName(QLatin1String("DForD Software"));
    QCoreApplication::setApplicationName(QLatin1String("imchenwen"));
    QCoreApplication::setApplicationVersion(QLatin1String("1.0"));
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

    QApplication a(argc, argv);


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
    QDir localeDir(localeDirPath);
    QStringList filters;
    filters << "*_" + locale + ".qm";
    QFileInfoList fil = localeDir.entryInfoList(filters, QDir::Files);
    foreach(const QFileInfo& fi, fil)
    {
        bool b = translator.load("imchenwen_zh_CN.qm", localeDirPath); // always load simplified chinese translation file
        if (!b)
        {
            qDebug() << "loading " << fi.fileName() << " from " << localeDirPath << " failed";
        }
        else
        {
            qDebug() << "loading " << fi.fileName() << " from " << localeDirPath << " success";
        }
    }
    bool b = a.installTranslator(&translator);
    if (!b)
    {
        qDebug() << "installing translator failed ";
    }

    // qt locale
    qtTranslator.load("qt_" + locale, localeDirPath + "/qt");

    a.installTranslator(&qtTranslator);

#if defined(Q_OS_MAC)
    a.setWindowIcon(QIcon(QLatin1String(":imchenwen.icns")));
#else
    a.setWindowIcon(QIcon(QLatin1String(":imchenwen.ico")));
#endif
    QtWebEngine::initialize();

    Browser& browser = Browser::instance();
    browser.loadSettings();
    BrowserWindow *window = browser.mainWindow();

    const QString url = getCommandLineUrlArgument();
    if (!url.isEmpty())
        window->loadPage(url);
    else
        window->loadHomePage();

    return a.exec();
}
