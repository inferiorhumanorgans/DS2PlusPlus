CONFIG += testcase

QT       -= gui
QT       += testlib sql

TARGET = tst_ihka46_ident
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lds2
INCLUDEPATH += ../../../../libds2
LIBPATH += ../../../../libds2

SOURCES += ihka46_ident.cpp
HEADERS += ihka46_ident.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
OTHER_FILES +=
