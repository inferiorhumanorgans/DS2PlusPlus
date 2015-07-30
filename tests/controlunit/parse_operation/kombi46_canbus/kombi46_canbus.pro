CONFIG += testcase

QT       -= gui
QT       += testlib sql

TARGET = tst_kombi46_canbus
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lds2
INCLUDEPATH += ../../../../libds2
LIBPATH += ../../../../libds2

SOURCES += kombi46_canbus.cpp
HEADERS += kombi46_canbus.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
OTHER_FILES +=
