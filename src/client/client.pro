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
    config.h

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
    config.cpp

FORMS += \
    certificateerrordialog.ui \
    passworddialog.ui \
    aboutdialog.ui \
    optiondialog.ui

RESOURCES += res/imchenwen.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/webenginewidgets/imchenwen
INSTALLS += target
