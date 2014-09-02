CONFIG += testcase

QT       -= gui
QT       += serialport testlib sql

TARGET = tst_kombi46_canbus
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lds2
INCLUDEPATH += $$top_srcdir/libds2
LIBPATH += $$top_builddir/libds2

SOURCES += kombi46_canbus.cpp
HEADERS += kombi46_canbus.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
OTHER_FILES +=
