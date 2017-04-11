TEMPLATE = app
TARGET = imchenwen
QT += webengine webenginewidgets xml concurrent
CONFIG += c++11

include(Boost.pri)

INCLUDEPATH += $$PWD/httpserver

HEADERS += $$PWD/httpserver/*.hpp \
    browser.h \
    browserwindow.h \
    tabwidget.h \
    urllineedit.h \
    webview.h \
    webpage.h \
    webpopupwindow.h \
    aboutdialog.h \
    config.h \
    waitingspinnerwidget.h \
    linkresolver.h \
    websites.h \
    playdialog.h \
    settings.h \
    streammanager.h \
    streamreply.h

SOURCES += $$PWD/httpserver/*.cpp \
    browser.cpp \
    browserwindow.cpp \
    main.cpp \
    tabwidget.cpp \
    urllineedit.cpp \
    webview.cpp \
    webpage.cpp \
    webpopupwindow.cpp \
    aboutdialog.cpp \
    config.cpp \
    waitingspinnerwidget.cpp \
    linkresolver.cpp \
    websites.cpp \
    playdialog.cpp \
    settings.cpp \
    streammanager.cpp \
    streamreply.cpp

FORMS += \
    certificateerrordialog.ui \
    passworddialog.ui \
    aboutdialog.ui \
    playdialog.ui \
    settings.ui

RESOURCES += res/imchenwen.qrc


RC_FILE = imchenwen-win.rc

macx: {
    ICON = res/imchenwen.icns
    icon.files += res/imchenwen128.png
    LIBS += -lboost_system
    CONFIG(release, debug|release) : {
        QMAKE_INFO_PLIST = osxInfo.plist
        MACDEPLOYQT = $$[QT_INSTALL_BINS]/macdeployqt

        deploy.commands += $$MACDEPLOYQT \"$${OUT_PWD}/$${TARGET}.app\" -appstore-compliant

        deploy_webengine.depends += deploy
        deploy_webengine.commands += $$MACDEPLOYQT \"$${OUT_PWD}/$${TARGET}.app/Contents/Frameworks/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app\" -appstore-compliant

        deploy_sniff.commands += $(COPY_FILE) \"$${PWD}/../parsed/sniff.js\" \"$${OUT_PWD}/$${TARGET}.app/Contents/Resources/\"

        build_parsed.commands += cd $${PWD}/../parsed/ $$escape_expand(&&) go build

        deploy_parsed.depends += build_parsed deploy_sniff
        deploy_parsed.commands += $(COPY_FILE) $${PWD}/../parsed/parsed \"$${OUT_PWD}/$${TARGET}.app/Contents/Resources/\"

        APPCERT = Developer ID Application: Fan Yang (Y73SBCN2CG)
        INSTALLERCERT = 3rd Party Mac Developer Installer: Fan Yang (Y73SBCN2CG)
        BUNDLEID = com.dfordsoft.imchenwen

        codesign.depends += deploy_webengine deploy_parsed
        codesign.commands = codesign -s \"$${APPCERT}\" -v -f --timestamp=none --deep \"$${OUT_PWD}/$${TARGET}.app\"

        makedmg.depends += codesign
        makedmg.commands = hdiutil create -srcfolder \"$${TARGET}.app\" -volname \"$${TARGET}\" -format UDBZ \"$${TARGET}.dmg\" -ov -scrub -stretch 2g

        QMAKE_EXTRA_TARGETS += deploy deploy_webengine deploy_sniff build_parsed deploy_parsed codesign makedmg
    }
}

win32: {
	DEFINES += _WIN32_WINNT=0x0600
    CONFIG(release, debug|release) : {
        LIBS += -llibboost_system-vc140-mt-1_63
		win32-msvc* {
			QMAKE_CXXFLAGS += /Zi
			QMAKE_LFLAGS += /INCREMENTAL:NO /Debug
		}
        WINDEPLOYQT = $$[QT_INSTALL_BINS]/windeployqt.exe

        deploy.commands += $$MACDEPLOYQT \"$${OUT_PWD}\\release\\$${TARGET}.exe\"

        build_parsed.commands += cd \"$${PWD}/../parsed/\" $$escape_expand(&&) go build

        deploy_parsed.depends += build_parsed
        deploy_parsed.commands += $(COPY_FILE) \"$${PWD}\\..\\parsed\\parsed.exe\" \"$${OUT_PWD}\\release\\parser\\parsed.exe\"

        build_updater.commands += cd \"$${PWD}/../updater/\" $$escape_expand(&&) go build

        deploy_updater.depends += build_updater
        deploy_updater.commands += $(COPY_FILE) \"$${PWD}\\..\\updater\\updater.exe\" \"$${OUT_PWD}\\release\\parser\\updater.exe\"
		
        deploy_sniff.commands += $(COPY_FILE) \"$${PWD}\\..\\parsed\\sniff.js\" \"$${OUT_PWD}\\release\\parser\\sniff.js\"

        all.depends += deploy_updater deploy_parsed deploy_sniff
        all.commands +=
        QMAKE_EXTRA_TARGETS += deploy build_parsed build_updater deploy_parsed deploy_updater deploy_sniff all
    }
}
