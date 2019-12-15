CONFIG *= depend_includepath

INCLUDEPATH += \
    $$PWD/include $$PWD/src

HEADERS += \
    $$PWD/include/qhttpengine/basicauthmiddleware.h \
    $$PWD/include/qhttpengine/filesystemhandler.h \
    $$PWD/include/qhttpengine/handler.h \
    $$PWD/include/qhttpengine/ibytearray.h \
    $$PWD/include/qhttpengine/localauthmiddleware.h \
    $$PWD/include/qhttpengine/localfile.h \
    $$PWD/include/qhttpengine/middleware.h \
    $$PWD/include/qhttpengine/parser.h \
    $$PWD/include/qhttpengine/proxyhandler.h \
    $$PWD/include/qhttpengine/qhttpengine_export.h \
    $$PWD/include/qhttpengine/qiodevicecopier.h \
    $$PWD/include/qhttpengine/qobjecthandler.h \
    $$PWD/include/qhttpengine/range.h \
    $$PWD/include/qhttpengine/server.h \
    $$PWD/include/qhttpengine/socket.h \
    $$PWD/src/basicauthmiddleware_p.h \
    $$PWD/src/filesystemhandler_p.h \
    $$PWD/src/handler_p.h \
    $$PWD/src/localauthmiddleware_p.h \
    $$PWD/src/localfile_p.h \
    $$PWD/src/proxyhandler_p.h \
    $$PWD/src/proxysocket.h \
    $$PWD/src/qiodevicecopier_p.h \
    $$PWD/src/qobjecthandler_p.h \
    $$PWD/src/range_p.h \
    $$PWD/src/server_p.h \
    $$PWD/src/socket_p.h

SOURCES += \
    $$PWD/src/basicauthmiddleware.cpp \
    $$PWD/src/filesystemhandler.cpp \
    $$PWD/src/handler.cpp \
    $$PWD/src/localauthmiddleware.cpp \
    $$PWD/src/localfile.cpp \
    $$PWD/src/parser.cpp \
    $$PWD/src/proxyhandler.cpp \
    $$PWD/src/proxysocket.cpp \
    $$PWD/src/qiodevicecopier.cpp \
    $$PWD/src/qobjecthandler.cpp \
    $$PWD/src/range.cpp \
    $$PWD/src/server.cpp \
    $$PWD/src/socket.cpp

win32 : {
    LIBS += -lAdvapi32
}
