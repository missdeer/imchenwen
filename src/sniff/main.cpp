#include <QApplication>
#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QTimer>
#include <tuple>
#include "urlrequestinterceptor.h"

std::tuple<bool, QUrl> commandLineUrlArgument()
{
    const QStringList args = QCoreApplication::arguments();
    QUrl u("https://minidump.info/imchenwen/");
    bool mpv = false;
    for (const QString &arg : args.mid(1)) {
        if (!arg.startsWith(QLatin1Char('-')))
            u = QUrl::fromUserInput(arg);
        if (arg == "--mpv")
            mpv = true;
    }
    return std::make_tuple(mpv, u);
}

void loadSettings(UrlRequestInterceptor& urlRequestInterceptor)
{
    QWebEngineSettings *defaultSettings = QWebEngineSettings::globalSettings();
    QWebEngineProfile *defaultProfile = QWebEngineProfile::defaultProfile();

    defaultSettings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    defaultSettings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);
    defaultSettings->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    defaultSettings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);

    defaultProfile->setHttpUserAgent("Mozilla/5.0 (Windows NT 6.2; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/65.0.3325.230 Safari/537.36");
    defaultProfile->setHttpAcceptLanguage("en-US");

    defaultSettings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, true);
    defaultSettings->setAttribute(QWebEngineSettings::TouchIconsEnabled, true);

    defaultProfile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);

    defaultProfile->setRequestInterceptor(&urlRequestInterceptor);
}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(QLatin1String("DForD Software"));
    QCoreApplication::setApplicationName(QObject::tr("sniff"));
    QCoreApplication::setApplicationVersion(QLatin1String("1.0"));
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    QApplication a(argc, argv);

    auto args = commandLineUrlArgument();
    UrlRequestInterceptor urlRequestInterceptor(std::get<0>(args));
    loadSettings(urlRequestInterceptor);

    QWebEngineView view;
    view.setUrl(std::get<1>(args));

    QTimer::singleShot(30 * 1000, [](){
        qApp->exit(1);
    });

    return a.exec();
}
