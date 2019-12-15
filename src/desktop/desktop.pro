TEMPLATE = app
TARGET = imchenwen
QT += core gui widgets network xml concurrent gui-private
CONFIG += c++17

contains(QMAKE_HOST.arch, x86_64): {
    DESTDIR = $$PWD/../../bin/x86_64
} else: {
    DESTDIR = $$PWD/../../bin/x86
}

include(Boost.pri)
include($$PWD/../3rdparty/qhttpengine/qhttpengine.pri)

INCLUDEPATH += $$PWD \
    $$PWD/dlna \
    $$PWD/mpv \
    $$PWD/config \
    $$PWD/ui \
    $$PWD/mediahandler \
    $$PWD/upgrade \
    $$PWD/network \
    $$PWD/storage

HEADERS += \
    $$PWD/util.h \
    $$PWD/config/config.h \
    $$PWD/config/player.h \
    $$PWD/config/subscriptionhelper.h \
    $$PWD/config/websites.h \
    $$PWD/dlna/DLNAPlaybackInfo.h \
    $$PWD/dlna/dlnaplayerview.h \
    $$PWD/dlna/DLNARenderer.h \
    $$PWD/dlna/DLNARendererIcon.h \
    $$PWD/dlna/Kast.h \
    $$PWD/dlna/MimeGuesser.h \
    $$PWD/dlna/SOAPActionManager.h \
    $$PWD/dlna/SSDPDiscovery.h \
    $$PWD/mediahandler/inmemoryhandler.h \
    $$PWD/mediahandler/linkresolver.h \
    $$PWD/mediahandler/linkresolverprocess.h \
    $$PWD/mediahandler/mediarelay.h \
    $$PWD/mediahandler/sniffer.h \
    $$PWD/mediahandler/vipresolver.h \
    $$PWD/mpv/playercore.h \
    $$PWD/mpv/playerview.h \
    $$PWD/mpv/skin.h \
    $$PWD/ui/browserwindow.h \
    $$PWD/ui/inputurldialog.h \
    $$PWD/ui/playdialog.h \
    $$PWD/ui/popupmenutoolbutton.h \
    $$PWD/ui/settings.h \
    $$PWD/ui/tabwidget.h \
    $$PWD/ui/urllineedit.h \
    $$PWD/ui/waitingspinnerwidget.h \
    $$PWD/browser.h \
    $$PWD/ui/donatedialog.h \
    $$PWD/upgrade/dependenciesupgrade.h \
    $$PWD/network/networkreplyhelper.h \
    $$PWD/mediahandler/yougetprocess.h \
    $$PWD/mediahandler/annieprocess.h \
    $$PWD/mediahandler/ykdlprocess.h \
    $$PWD/mediahandler/youtubedlprocess.h \
    $$PWD/network/proxyrule.h \
    $$PWD/network/outofchinamainlandproxyfactory.h \
    $$PWD/network/ingfwlistproxyfactory.h \
    $$PWD/storage/storageservice.h \
    $$PWD/mediahandler/streaminfo.h \
    $$PWD/mediahandler/mediainfo.h \
    $$PWD/mediahandler/subtitleinfo.h

SOURCES += \
    $$PWD/browser.cpp \
    $$PWD/main.cpp \
    $$PWD/util.cpp \
    $$PWD/config/config.cpp \
    $$PWD/config/player.cpp \
    $$PWD/config/subscriptionhelper.cpp \
    $$PWD/config/websites.cpp \
    $$PWD/dlna/dlnaplayerview.cpp \
    $$PWD/dlna/DLNARenderer.cpp \
    $$PWD/dlna/Kast.cpp \
    $$PWD/dlna/MimeGuesser.cpp \
    $$PWD/dlna/SOAPActionManager.cpp \
    $$PWD/dlna/SSDPDiscovery.cpp \
    $$PWD/mediahandler/inmemoryhandler.cpp \
    $$PWD/mediahandler/linkresolver.cpp \
    $$PWD/mediahandler/linkresolverprocess.cpp \
    $$PWD/mediahandler/mediarelay.cpp \
    $$PWD/mediahandler/sniffer.cpp \
    $$PWD/mediahandler/vipresolver.cpp \
    $$PWD/mpv/playercore.cpp \
    $$PWD/mpv/playerview.cpp \
    $$PWD/mpv/skin.cpp \
    $$PWD/ui/browserwindow.cpp \
    $$PWD/ui/inputurldialog.cpp \
    $$PWD/ui/playdialog.cpp \
    $$PWD/ui/popupmenutoolbutton.cpp \
    $$PWD/ui/settings.cpp \
    $$PWD/ui/tabwidget.cpp \
    $$PWD/ui/urllineedit.cpp \
    $$PWD/ui/waitingspinnerwidget.cpp \
    $$PWD/ui/donatedialog.cpp \
    $$PWD/upgrade/dependenciesupgrade.cpp \
    $$PWD/network/networkreplyhelper.cpp \
    $$PWD/mediahandler/yougetprocess.cpp \
    $$PWD/mediahandler/annieprocess.cpp \
    $$PWD/mediahandler/ykdlprocess.cpp \
    $$PWD/mediahandler/youtubedlprocess.cpp \
    $$PWD/network/proxyrule.cpp \
    $$PWD/network/outofchinamainlandproxyfactory.cpp \
    $$PWD/network/ingfwlistproxyfactory.cpp \
    $$PWD/storage/storageservice.cpp \
    $$PWD/mediahandler/streaminfo.cpp \
    $$PWD/mediahandler/mediainfo.cpp \
    $$PWD/mediahandler/subtitleinfo.cpp

FORMS += \
    $$PWD/dlna/dlnaplayerview.ui \
    $$PWD/mpv/playerview.ui \
    $$PWD/ui/certificateerrordialog.ui \
    $$PWD/ui/inputurldialog.ui \
    $$PWD/ui/passworddialog.ui \
    $$PWD/ui/playdialog.ui \
    $$PWD/ui/settings.ui \
    $$PWD/ui/donatedialog.ui

RESOURCES += $$PWD/res/imchenwen.qrc $$PWD/res/icons.qrc

RC_FILE = $$PWD/imchenwen-win.rc

CODECFORTR      = UTF-8
CODECFORSRC     = UTF-8
TRANSLATIONS    = $$PWD/translations/imchenwen_en_US.ts \
                  $$PWD/translations/imchenwen_zh_CN.ts

isEmpty(QMAKE_LUPDATE) {
    win32:QMAKE_LUPDATE = $$[QT_INSTALL_BINS]\lupdate.exe
    else:QMAKE_LUPDATE = $$[QT_INSTALL_BINS]/lupdate
}

isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}

lupdate.commands = $$QMAKE_LUPDATE -no-obsolete $$PWD/desktop.pro
lupdates.depends = $$SOURCES $$HEADERS $$FORMS $$TRANSLATIONS
lrelease.commands = $$QMAKE_LRELEASE $$PWD/desktop.pro
lrelease.depends = lupdate
translate.depends = lrelease
QMAKE_EXTRA_TARGETS += lupdate lrelease translate qti18n
POST_TARGETDEPS += translate qti18n

macx: {
    QT += macextras
    ICON = res/imchenwen.icns
    icon.files += res/imchenwen128.png
    LIBS+=-framework Cocoa -framework WebKit
    HEADERS  += \
        $$PWD/cocoawebview/cocoawebview.h \
        $$PWD/cocoawebview/qtcocoawebview.h

    OBJECTIVE_SOURCES += \
        $$PWD/cocoawebview/cocoawebview.mm \
        $$PWD/cocoawebview/qtcocoawebview.mm

    INCLUDEPATH += $$PWD/cocoawebview

    CONFIG(release, debug|release) : {
        QMAKE_INFO_PLIST = osxInfo.plist
        MACDEPLOYQT = $$[QT_INSTALL_BINS]/macdeployqt

        translate.depends = lrelease
        translate.files = $$system("find $${PWD}/translations -name '*.qm' ")
        translate.path = Contents/Resources/translations/
        translate.commands = '$(COPY_DIR) $$shell_path($${PWD}/translations) $$shell_path($${DESTDIR}/$${TARGET}.app/Contents/Resources/)'

        qti18n.depends = translate
        qti18n.commands = '$(COPY_FILE) $$shell_path($$[QT_INSTALL_BINS]/../translations/qt_zh_CN.qm) $$shell_path($${DESTDIR}/$${TARGET}.app/Contents/Resources/translations/qt_zh_CN.qm)'

        bundlei18n.depends = qti18n
        bundlei18n.commands = '$(COPY_DIR) $$shell_path($${PWD}/../../mac/*.lproj) $$shell_path($${DESTDIR}/$${TARGET}.app/Contents/Resources/)'
        QMAKE_BUNDLE_DATA += translate qti18n bundlei18n

        deploy.commands += $$MACDEPLOYQT \"$${DESTDIR}/$${TARGET}.app\"

        deploy_appstore.depends += deploy
        deploy_appstore.commands += $$MACDEPLOYQT \"$${DESTDIR}/$${TARGET}.app\" -appstore-compliant

        deploy_webengine.depends += deploy_appstore
        deploy_webengine.commands += $$MACDEPLOYQT \"$${DESTDIR}/$${TARGET}.app/Contents/Frameworks/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app\"

        fixdeploy.depends += deploy_webengine
        fixdeploy.commands += $$PWD/../macdeploy/macdeploy \"$${DESTDIR}/$${TARGET}.app\"

        APPCERT = Developer ID Application: Fan Yang (Y73SBCN2CG)
        INSTALLERCERT = 3rd Party Mac Developer Installer: Fan Yang (Y73SBCN2CG)
        BUNDLEID = com.dfordsoft.imchenwen

        codesign.depends += fixdeploy
        codesign.commands = codesign -s \"$${APPCERT}\" -v -f --timestamp=none --deep \"$${DESTDIR}/$${TARGET}.app\"

        makedmg.depends += codesign
        makedmg.commands = hdiutil create -srcfolder \"$${DESTDIR}/$${TARGET}.app\" -volname \"$${TARGET}\" -format UDBZ \"$${DESTDIR}/$${TARGET}.dmg\" -ov -scrub -stretch 2g

        QMAKE_EXTRA_TARGETS += deploy deploy_webengine deploy_appstore fixdeploy codesign makedmg bundlei18n
        POST_TARGETDEPS += bundlei18n
    }
} else: {
    QT += webengine webenginecore webenginewidgets 
    SOURCES += \
        $$PWD/webengine/webpage.cpp \
        $$PWD/webengine/webpopupwindow.cpp \
        $$PWD/webengine/webview.cpp 
    
    INCLUDEPATH += \
        $$PWD/webengine 

    HEADERS += \
        $$PWD/webengine/webpage.h \
        $$PWD/webengine/webpopupwindow.h \
        $$PWD/webengine/webview.h 
}

win32: {
    DEFINES += _WIN32_WINNT=0x0600 BOOST_ALL_NO_LIB=1

    CONFIG(release, debug|release) : {
        #QMAKE_CXXFLAGS += /Zi
        #QMAKE_LFLAGS += /INCREMENTAL:NO /Debug
        WINDEPLOYQT = $$[QT_INSTALL_BINS]/windeployqt.exe
    }
    translate.commands = '$(COPY_DIR) $$shell_path($$PWD/translations) $$shell_path($$DESTDIR/translations)'

    INCLUDEPATH += $$PWD/../3rdparty/libmpv/include
    contains(QMAKE_HOST.arch, x86_64): {
        LIBS += -L$$PWD/../3rdparty/libmpv/x86_64
        copy_mpv_dll.commands = '$(COPY_FILE) $$shell_path($$PWD/../3rdparty/libmpv/x86_64/mpv-1.dll) $$shell_path($$DESTDIR/mpv-1.dll)'
        copy_innosetup.commands = '$(COPY_FILE) $$shell_path($$PWD/imchenwen-msvc-x64.iss) $$shell_path($$DESTDIR/imchenwen-msvc-x64.iss)'
    } else : {
        LIBS += -L$$PWD/../3rdparty/libmpv/i686
        copy_mpv_dll.commands = '$(COPY_FILE) $$shell_path($$PWD/../3rdparty/libmpv/i686/mpv-1.dll) $$shell_path($$DESTDIR/mpv-1.dll)'
        copy_innosetup.commands = '$(COPY_FILE) $$shell_path($$PWD/imchenwen-msvc-x86.iss) $$shell_path($$DESTDIR/imchenwen-msvc-x86.iss)'
    }

    qti18n.depends = translate
    qti18n.commands = '$(COPY_FILE) $$shell_path($$[QT_INSTALL_BINS]/../translations/qt_zh_CN.qm) $$shell_path($${DESTDIR}/translations/qt_zh_CN.qm)'

    QMAKE_EXTRA_TARGETS += copy_mpv_dll copy_innosetup
    POST_TARGETDEPS += copy_mpv_dll copy_innosetup
} else : {
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib
}

LIBS += -lmpv

unix: !macx {
    translate.commands = '$(COPY_DIR) $$shell_path($$PWD/translations) $$shell_path($$DESTDIR)'
}
