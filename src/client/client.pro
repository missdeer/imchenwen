TEMPLATE = app
TARGET = imchenwen
QT += webengine webenginewidgets xml
CONFIG += c++11

HEADERS += \
    browser.h \
    browserwindow.h \
    tabwidget.h \
    urllineedit.h \
    webview.h \
    webpage.h \
    webpopupwindow.h \
    aboutdialog.h \
    optiondialog.h \
    config.h \
    waitingspinnerwidget.h \
    linkresolver.h \
    websites.h \
    externalplay.h \
    externalplaydialog.h

SOURCES += \
    browser.cpp \
    browserwindow.cpp \
    main.cpp \
    tabwidget.cpp \
    urllineedit.cpp \
    webview.cpp \
    webpage.cpp \
    webpopupwindow.cpp \
    aboutdialog.cpp \
    optiondialog.cpp \
    config.cpp \
    waitingspinnerwidget.cpp \
    linkresolver.cpp \
    websites.cpp \
    externalplay.cpp \
    externalplaydialog.cpp

FORMS += \
    certificateerrordialog.ui \
    passworddialog.ui \
    aboutdialog.ui \
    optiondialog.ui \
    externalplaydialog.ui

RESOURCES += res/imchenwen.qrc


RC_FILE = imchenwen-win.rc

macx: {
    ICON = res/imchenwen.icns
    icon.files += res/imchenwen128.png

    CONFIG(release, debug|release) : {
        QMAKE_INFO_PLIST = osxInfo.plist
        MACDEPLOYQT = $$[QT_INSTALL_BINS]/macdeployqt

        deploy.commands += $$MACDEPLOYQT \"$${OUT_PWD}/$${TARGET}.app\" -appstore-compliant

        deploywebengine.depends += deploy
        deploywebengine.commands += $$MACDEPLOYQT \"$${OUT_PWD}/$${TARGET}.app/Contents/Frameworks/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app\" -appstore-compliant

        APPCERT = Developer ID Application: Fan Yang (Y73SBCN2CG)
        INSTALLERCERT = 3rd Party Mac Developer Installer: Fan Yang (Y73SBCN2CG)
        BUNDLEID = com.dfordsoft.imchenwen

        codesign_bundle.depends += deploywebengine
        codesign_bundle.commands = codesign -s \"$${APPCERT}\" -v -f --timestamp=none --deep \"$${OUT_PWD}/$${TARGET}.app\"
        makedmg.depends += codesign_bundle
        makedmg.commands = hdiutil create -srcfolder \"$${TARGET}.app\" -volname \"$${TARGET}\" -format UDBZ \"$${TARGET}.dmg\" -ov -scrub -stretch 2g
        QMAKE_EXTRA_TARGETS += deploy codesign_bundle makedmg
    }
}

