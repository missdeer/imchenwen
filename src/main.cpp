/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <clocale>

#include <QCoreApplication>
#include <QDir>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSettings>
#include <QTimer>
#include <QTranslator>

#include "accessManager.h"
#include "application.h"
#include "downloader.h"
#include "platform/graphics.h"
#include "platform/paths.h"
#include "plugin.h"

int main(int argc, char *argv[])
{
    // Check Qt version
    if (strcmp(qVersion(), QT_VERSION_STR) != 0)
    {
        qWarning("The program is compiled against Qt %s but running on Qt %s,"
                 "which may lead to unexpected behaviors.",
                 QT_VERSION_STR,
                 qVersion());
    }

    // Force to use OpenGL in Qt6
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi);

    // Set application attributes
    QCoreApplication::setOrganizationName(QStringLiteral("DForD Software"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("dfordsoft.com"));
    QCoreApplication::setApplicationName(QStringLiteral("imchenwen"));
    QCoreApplication::setApplicationVersion(QStringLiteral(IMCHENWEN_VERSION));

    // Detect OpenGL, stage 1
    Graphics::detectOpenGLEarly();

    // Create application instance
    Application app(argc, argv);

    // Check whether another instance is running
    if (app.connectAnotherInstance())
    {
        app.sendFileLists();
        return 0;
    }

    // Create and listen local server
    app.createServer();

    // Detect OpenGL, stage 2
    Graphics::detectOpenGLLate();

    // Qt sets the locale in the QGuiApplication constructor, but libmpv
    // requires the LC_NUMERIC category to be set to "C", so change it back.
    std::setlocale(LC_NUMERIC, "C");
    qputenv("LC_NUMERIC", QByteArrayLiteral("C"));
    qputenv("PYTHONIOENCODING", QByteArrayLiteral("utf-8"));

    // Translate
    QTranslator translator;
    if (translator.load(QStringLiteral("imchenwen_") + QLocale::system().name(), QStringLiteral(":/l10n")))
    {
        QCoreApplication::installTranslator(&translator);
    }
    QTranslator qttranslator;
    if (qttranslator.load(QStringLiteral("qt_") + QLocale::system().name(), QCoreApplication::applicationDirPath() + QStringLiteral("/translations")))
    {
        QCoreApplication::installTranslator(&qttranslator);
    }

    QQmlApplicationEngine engine;
    engine.addImportPath(QCoreApplication::applicationDirPath() + QStringLiteral("/qml"));
    engine.addImportPath(QStringLiteral("qrc:/"));

    // Set UI style
    switch (QSettings().value(QStringLiteral("player/theme"), 1).toInt())
    {
    case 0: // Classic
        break;

    case 1: // Material
        qputenv("QT_QUICK_CONTROLS_MATERIAL_VARIANT", QByteArrayLiteral("Dense"));
        qputenv("QT_QUICK_CONTROLS_STYLE", QByteArrayLiteral("Material"));
        break;

    case 2: // Win10
        qputenv("QT_QUICK_CONTROLS_STYLE", QByteArrayLiteral("Universal"));
        break;
    }

    QQmlContext *context    = engine.rootContext();
    Downloader  *downloader = Downloader::instance();

    Q_ASSERT(context != nullptr);
    Q_ASSERT(downloader != nullptr);

    context->setContextProperty(QStringLiteral("accessManager"), NetworkAccessManager::instance());
    context->setContextProperty(QStringLiteral("downloaderModel"), QVariant::fromValue(downloader->model()));
    context->setContextProperty(QStringLiteral("plugins"), QVariant::fromValue(Plugin::loadPlugins()));

    // Update downloader model
    QObject::connect(downloader, &Downloader::modelUpdated, [context, downloader]() {
        context->setContextProperty(QStringLiteral("downloaderModel"), QVariant::fromValue(downloader->model()));
    });

    engine.load(QUrl(QStringLiteral("qrc:/com/dfordsoft/imchenwen/qml/main.qml")));

    // Create user resources dir
    if (!QDir(userResourcesPath()).exists())
    {
        QDir().mkpath(userResourcesPath());
    }

    // Open files from arguments
    // Wait 0.5s to ensure OpenGL is loaded
    QTimer::singleShot(500, [&]() { app.processFileLists(); });

    return QCoreApplication::exec();
}
