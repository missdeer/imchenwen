cache()

TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += src/otter/otter.pro src/manager/manager.pro

!win32-*g++* {
    SUBDIRS += src/client/client.pro
}
