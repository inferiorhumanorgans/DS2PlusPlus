CONFIG += testcase

QT       -= gui
QT       += testlib sql

TARGET = tst_zke5_ident
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lds2
INCLUDEPATH += ../../../../libds2
LIBPATH += ../../../../libds2

SOURCES += zke5_ident.cpp
HEADERS += zke5_ident.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
OTHER_FILES +=
