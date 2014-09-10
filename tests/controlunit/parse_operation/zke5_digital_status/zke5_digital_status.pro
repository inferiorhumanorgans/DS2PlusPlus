CONFIG += testcase

QT       -= gui
QT       += testlib sql

TARGET = tst_zke5_digital_status
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lds2
INCLUDEPATH += $$top_srcdir/libds2
LIBPATH += $$top_builddir/libds2

SOURCES += zke5_digital_status.cpp
HEADERS += zke5_digital_status.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
OTHER_FILES +=
