#include "browser.h"
#include "browserwindow.h"
#include <QApplication>
#include <QtWebEngine>
#include <QWebEngineSettings>

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

    QApplication app(argc, argv);
#if defined(Q_OS_MAC)
    app.setWindowIcon(QIcon(QLatin1String(":imchenwen.icns")));
#else
    app.setWindowIcon(QIcon(QLatin1String(":imchenwen.ico")));
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

    return app.exec();
}
