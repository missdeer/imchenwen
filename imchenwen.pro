cache()

TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += src/client/client.pro src/manager/manager.pro

win32-msvc* {
    QMAKE_CXXFLAGS += /Zi
    QMAKE_LFLAGS += /INCREMENTAL:NO /Debug
}
