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
    playdialog.h \
    playurldialog.h

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
    playdialog.cpp \
    playurldialog.cpp

FORMS += \
    certificateerrordialog.ui \
    passworddialog.ui \
    aboutdialog.ui \
    optiondialog.ui \
    playdialog.ui \
    playurldialog.ui

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
        deploy_webengine.commands += $$MACDEPLOYQT \"$${OUT_PWD}/$${TARGET}.app/Contents/Frameworks/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app\" -appstore-compliant

        build_parsed.commands += cd $${PWD}/../parsed/ $$escape_expand(&&) go build

        deploy_parsed.depends += build_parsed
        deploy_parsed.commands += $(COPY_FILE) $${PWD}/../parsed/parsed \"$${OUT_PWD}/$${TARGET}.app/Contents/Resources/\"

        APPCERT = Developer ID Application: Fan Yang (Y73SBCN2CG)
        INSTALLERCERT = 3rd Party Mac Developer Installer: Fan Yang (Y73SBCN2CG)
        BUNDLEID = com.dfordsoft.imchenwen

        codesign.depends += deploy_webengine deploy_parsed
        codesign.commands = codesign -s \"$${APPCERT}\" -v -f --timestamp=none --deep \"$${OUT_PWD}/$${TARGET}.app\"

        makedmg.depends += codesign
        makedmg.commands = hdiutil create -srcfolder \"$${TARGET}.app\" -volname \"$${TARGET}\" -format UDBZ \"$${TARGET}.dmg\" -ov -scrub -stretch 2g

        QMAKE_EXTRA_TARGETS += deploy deploy_webengine build_parsed deploy_parsed codesign makedmg
    }
}

