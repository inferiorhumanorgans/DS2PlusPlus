#-------------------------------------------------
#
# Project created by QtCreator 2015-08-07T11:19:18
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_initializationtest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_initializationtest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

LIBS += -lds2
INCLUDEPATH += ../../../libds2
LIBPATH += ../../../libds2
