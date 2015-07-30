CONFIG += testcase

QT       -= gui
QT       += testlib sql

TARGET = tst_dme_ms420_ident
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lds2
INCLUDEPATH += ../../../../libds2
LIBPATH += ../../../..//libds2

SOURCES += dme_ms420_ident.cpp
HEADERS += dme_ms420_ident.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
OTHER_FILES +=
