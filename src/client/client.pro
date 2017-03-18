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
}

