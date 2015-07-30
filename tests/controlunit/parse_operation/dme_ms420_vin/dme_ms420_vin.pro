CONFIG += testcase

QT       -= gui
QT       += testlib sql

TARGET = tst_dme_ms420_vin
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lds2
INCLUDEPATH += ../../../../libds2
LIBPATH += ../../../../libds2

SOURCES += dme_ms420_vin.cpp
HEADERS += dme_ms420_vin.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
OTHER_FILES +=
