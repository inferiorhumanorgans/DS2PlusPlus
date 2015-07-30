CONFIG += testcase

QT       -= gui
QT       += testlib sql

TARGET = tst_lsz_status
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lds2
INCLUDEPATH += ../../../../libds2
LIBPATH += ../../../../libds2

SOURCES += lsz_status.cpp
HEADERS += lsz_status.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
OTHER_FILES +=
