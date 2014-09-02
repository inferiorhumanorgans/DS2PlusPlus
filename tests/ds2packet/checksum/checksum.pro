CONFIG += testcase

QT       -= gui
QT       += serialport testlib sql

TARGET = tst_ds2packet_checksum
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lds2
INCLUDEPATH += $$top_srcdir/libds2
LIBPATH += $$top_builddir/libds2

SOURCES += main.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
OTHER_FILES +=
