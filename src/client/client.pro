TEMPLATE = app
TARGET = imchenwen
QT += webengine webenginecore webenginewidgets xml concurrent
CONFIG += c++14

include(Boost.pri)
include($$PWD/../3rdparty/qhttpengine/qhttpengine.pri)

INCLUDEPATH += $$PWD \
    $$PWD/dlna \
    $$PWD/mpv \
    $$PWD/webengine \
    $$PWD/config \
    $$PWD/ui \
    $$PWD/mediahandler

HEADERS += $$PWD/dlna/*.h \
    $$PWD/mpv/*.h \
    $$PWD/webengine/*.h \
    $$PWD/browser.h \
    $$PWD/ui/*.h \
    $$PWD/config/*.h \
    $$PWD/mediahandler/*.h \
    $$PWD/util.h

SOURCES += $$PWD/dlna/*.cpp \
    $$PWD/mpv/*.cpp \
    $$PWD/webengine/*.cpp \
    $$PWD/ui/*.cpp \
    $$PWD/config/*.cpp \
    $$PWD/mediahandler/*.cpp \
    $$PWD/browser.cpp \
    $$PWD/main.cpp \
    $$PWD/util.cpp

FORMS += \
    $$PWD/ui/*.ui \
    $$PWD/mpv/*.ui \
    $$PWD/dlna/*.ui

RESOURCES += $$PWD/res/imchenwen.qrc $$PWD/res/icons.qrc

RC_FILE = $$PWD/imchenwen-win.rc

CODECFORTR      = UTF-8
CODECFORSRC     = UTF-8
TRANSLATIONS    = $$PWD/translations/imchenwen_en.ts \
                  $$PWD/translations/imchenwen_zh_CN.ts

isEmpty(QMAKE_LUPDATE) {
    win32:QMAKE_LUPDATE = $$[QT_INSTALL_BINS]\lupdate.exe
    else:QMAKE_LUPDATE = $$[QT_INSTALL_BINS]/lupdate
}

isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}

lupdate.commands = $$QMAKE_LUPDATE -no-obsolete $$PWD/client.pro
lupdates.depends = $$SOURCES $$HEADERS $$FORMS $$TRANSLATIONS
lrelease.commands = $$QMAKE_LRELEASE $$PWD/client.pro
lrelease.depends = lupdate
translate.depends = lrelease
QMAKE_EXTRA_TARGETS += lupdate lrelease translate
POST_TARGETDEPS += translate

macx: {
    QT += macextras
    ICON = res/imchenwen.icns
    icon.files += res/imchenwen128.png
    LIBS += -framework Cocoa -framework WebKit
    HEADERS  += \
        $$PWD/cocoawebview/*.h

    OBJECTIVE_SOURCES += \
        $$PWD/cocoawebview/*.mm

    INCLUDEPATH += $$PWD/cocoawebview

    CONFIG(release, debug|release) : {
        QMAKE_INFO_PLIST = osxInfo.plist
        MACDEPLOYQT = $$[QT_INSTALL_BINS]/macdeployqt

        translate.depends = lrelease
        translate.files = $$system("find $${PWD}/translations -name '*.qm' ")
        translate.path = Contents/Resources/translations/
        translate.commands = '$(COPY_DIR) $$shell_path($${PWD}/translations) $$shell_path($${OUT_PWD}/$${TARGET}.app/Contents/Resources/)'
        QMAKE_BUNDLE_DATA += translate

        deploy.commands += $$MACDEPLOYQT \"$${OUT_PWD}/$${TARGET}.app\"

        deploy_webengine.depends += deploy
        deploy_webengine.commands += $$MACDEPLOYQT \"$${OUT_PWD}/$${TARGET}.app/Contents/Frameworks/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app\"

        APPCERT = Developer ID Application: Fan Yang (Y73SBCN2CG)
        INSTALLERCERT = 3rd Party Mac Developer Installer: Fan Yang (Y73SBCN2CG)
        BUNDLEID = com.dfordsoft.imchenwen

        codesign.depends += deploy_webengine
        codesign.commands = codesign -s \"$${APPCERT}\" -v -f --timestamp=none --deep \"$${OUT_PWD}/$${TARGET}.app\"

        makedmg.depends += codesign
        makedmg.commands = hdiutil create -srcfolder \"$${TARGET}.app\" -volname \"$${TARGET}\" -format UDBZ \"$${TARGET}.dmg\" -ov -scrub -stretch 2g

        QMAKE_EXTRA_TARGETS += deploy deploy_webengine codesign makedmg
    }
}

win32: {
    DEFINES += _WIN32_WINNT=0x0600 BOOST_ALL_NO_LIB=1

    INCLUDEPATH += $$PWD/../3rdparty/libmpv/include
    contains(QMAKE_HOST.arch, x86_64): {
        LIBS += -L$$PWD/../3rdparty/libmpv/x86_64
        copy_mpv_dll.commands = '$(COPY_FILE) $$shell_path($$PWD/../3rdparty/libmpv/x86_64/mpv-1.dll) $$shell_path($$OUT_PWD/Release/mpv-1.dll)'
    } else : {
        LIBS += -L$$PWD/../3rdparty/libmpv/i686
        copy_mpv_dll.commands = '$(COPY_FILE) $$shell_path($$PWD/../3rdparty/libmpv/i686/mpv-1.dll) $$shell_path($$OUT_PWD/Release/mpv-1.dll)'
    }
    LIBS += -lmpv
    QMAKE_EXTRA_TARGETS += copy_mpv_dll
    POST_TARGETDEPS += copy_mpv_dll
    CONFIG(release, debug|release) : {
        QMAKE_CXXFLAGS += /Zi
        QMAKE_LFLAGS += /INCREMENTAL:NO /Debug
        WINDEPLOYQT = $$[QT_INSTALL_BINS]/windeployqt.exe
        translate.commands = '$(COPY_DIR) $$shell_path($$PWD/translations) $$shell_path($$OUT_PWD/release/translations)'
    } else: {
        translate.commands = '$(COPY_DIR) $$shell_path($$PWD/translations) $$shell_path($$OUT_PWD/debug/translations)'
    }
} else : {
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib -lmpv
}

unix: !macx {
    translate.commands = '$(COPY_DIR) $$shell_path($$PWD/translations) $$shell_path($$OUT_PWD)'
}
