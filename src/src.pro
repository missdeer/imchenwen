#-------------------------------------------------
#
# Project created by QtCreator 2013-07-17T13:16:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = imchenwen
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    aboutdialog.cpp \
    optiondialog.cpp

HEADERS  += mainwindow.h \
    aboutdialog.h \
    optiondialog.h

FORMS    += mainwindow.ui \
    aboutdialog.ui \
    optiondialog.ui

RESOURCES += \
    imchenwen.qrc

OTHER_FILES += \
    imchenwen-win.rc

RC_FILE = imchenwen-win.rc
