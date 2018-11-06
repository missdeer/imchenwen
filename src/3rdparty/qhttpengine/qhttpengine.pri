CONFIG *= depend_includepath

INCLUDEPATH += \
    $$PWD/include $$PWD/src

HEADERS += \
    $$PWD/include/qhttpengine/*.h \
    $$PWD/src/*.h

SOURCES += \
    $$PWD/src/*.cpp

win32 : {
    LIBS += -lAdvapi32
}
