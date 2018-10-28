TEMPLATE = app
TARGET = imchenwen
QT += webengine webenginecore webenginewidgets xml concurrent
CONFIG += c++11

include(Boost.pri)

INCLUDEPATH += $$PWD/httpserver

HEADERS += \
    browser.h \
    browserwindow.h \
    tabwidget.h \
    urllineedit.h \
    webview.h \
    webpage.h \
    webpopupwindow.h \
    config.h \
    waitingspinnerwidget.h \
    linkresolver.h \
    websites.h \
    playdialog.h \
    settings.h \
    linkresolverprocess.h \
    popupmenutoolbutton.h \
    urlrequestinterceptor.h

SOURCES += \
    browser.cpp \
    browserwindow.cpp \
    main.cpp \
    tabwidget.cpp \
    urllineedit.cpp \
    webview.cpp \
    webpage.cpp \
    webpopupwindow.cpp \
    config.cpp \
    waitingspinnerwidget.cpp \
    linkresolver.cpp \
    websites.cpp \
    playdialog.cpp \
    settings.cpp \
    linkresolverprocess.cpp \
    popupmenutoolbutton.cpp \
    urlrequestinterceptor.cpp

FORMS += \
    certificateerrordialog.ui \
    passworddialog.ui \
    playdialog.ui \
    settings.ui

RESOURCES += res/imchenwen.qrc


RC_FILE = imchenwen-win.rc

macx: {
    ICON = res/imchenwen.icns
    icon.files += res/imchenwen128.png

    CONFIG(release, debug|release) : {
        QMAKE_INFO_PLIST = osxInfo.plist
        MACDEPLOYQT = $$[QT_INSTALL_BINS]/macdeployqt

        deploy.commands += $$MACDEPLOYQT \"$${OUT_PWD}/$${TARGET}.app\" -appstore-compliant

        deploy_webengine.depends += deploy
        deploy_webengine.commands += # $$MACDEPLOYQT \"$${OUT_PWD}/$${TARGET}.app/Contents/Frameworks/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app\" -appstore-compliant

        APPCERT = Developer ID Application: Fan Yang (Y73SBCN2CG)
        INSTALLERCERT = 3rd Party Mac Developer Installer: Fan Yang (Y73SBCN2CG)
        BUNDLEID = com.dfordsoft.imchenwen

        codesign.depends += deploy_webengine deploy_parsed
        codesign.commands = codesign -s \"$${APPCERT}\" -v -f --timestamp=none --deep \"$${OUT_PWD}/$${TARGET}.app\"

        makedmg.depends += codesign
        makedmg.commands = hdiutil create -srcfolder \"$${TARGET}.app\" -volname \"$${TARGET}\" -format UDBZ \"$${TARGET}.dmg\" -ov -scrub -stretch 2g

        QMAKE_EXTRA_TARGETS += deploy deploy_webengine codesign makedmg
    }
}

win32: {
        DEFINES += _WIN32_WINNT=0x0600 BOOST_ALL_NO_LIB=1
        CONFIG(release, debug|release) : {

        win32-*msvc* {
                QMAKE_CXXFLAGS += /Zi
                QMAKE_LFLAGS += /INCREMENTAL:NO /Debug
        }
        WINDEPLOYQT = $$[QT_INSTALL_BINS]/windeployqt.exe
    }
}
