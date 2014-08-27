#-------------------------------------------------
#
# Project created by QtCreator 2014-08-24T14:29:21
#
#-------------------------------------------------

QT       -= gui
QT       += core serialport sql

TARGET = ds2-dump
CONFIG   += console
CONFIG   -= app_bundle

LIBS += -lds2
INCLUDEPATH += ../libds2
LIBPATH += ../libds2

TEMPLATE = app


SOURCES += main.cpp \
    ds2-dump.cpp

HEADERS += \
    ds2-dump.h
