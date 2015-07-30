CONFIG += testcase

QT       -= gui
QT       += testlib sql

TARGET = tst_kombi46_ident
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lds2
INCLUDEPATH += ../../../../libds2
LIBPATH += ../../../../libds2

SOURCES += kombi46_ident.cpp
HEADERS += kombi46_ident.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
OTHER_FILES +=
