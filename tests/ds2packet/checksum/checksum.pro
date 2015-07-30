CONFIG += testcase

QT       -= gui
QT       += testlib sql

TARGET = tst_ds2packet_checksum
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lds2
INCLUDEPATH += ../../../libds2
LIBPATH += ../../../libds2

SOURCES += main.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
OTHER_FILES +=
