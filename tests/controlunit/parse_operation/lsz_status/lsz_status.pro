CONFIG += testcase

QT       -= gui
QT       += serialport testlib sql

TARGET = tst_lsz_status
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lds2
INCLUDEPATH += $$top_srcdir/libds2
LIBPATH += $$top_builddir/libds2

SOURCES += lsz_status.cpp
HEADERS += lsz_status.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
OTHER_FILES +=
