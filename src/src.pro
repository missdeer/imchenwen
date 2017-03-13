TEMPLATE = app
TARGET = imchenwen
QT += webenginewidgets
CONFIG += c++11

HEADERS += \
    browser.h \
    browserwindow.h \
    tabwidget.h \
    urllineedit.h \
    webview.h \
    webpage.h \
    webpopupwindow.h

SOURCES += \
    browser.cpp \
    browserwindow.cpp \
    main.cpp \
    tabwidget.cpp \
    urllineedit.cpp \
    webview.cpp \
    webpage.cpp \
    webpopupwindow.cpp

FORMS += \
    certificateerrordialog.ui \
    passworddialog.ui

RESOURCES += data/imchenwen.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/webenginewidgets/imchenwen
INSTALLS += target
