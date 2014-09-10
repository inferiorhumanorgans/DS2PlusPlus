CONFIG += testcase

QT       -= gui
QT       += testlib sql

TARGET = tst_kombi46_ident
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lds2
INCLUDEPATH += $$top_srcdir/libds2
LIBPATH += $$top_builddir/libds2

SOURCES += kombi46_ident.cpp
HEADERS += kombi46_ident.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
OTHER_FILES +=
