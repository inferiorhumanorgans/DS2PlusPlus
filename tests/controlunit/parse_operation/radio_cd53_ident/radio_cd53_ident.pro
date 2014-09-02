CONFIG += testcase

QT       -= gui
QT       += serialport testlib sql

TARGET = tst_radio_cd53_ident
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lds2
INCLUDEPATH += $$top_srcdir/libds2
LIBPATH += $$top_builddir/libds2

SOURCES += radio_cd53_ident.cpp
HEADERS += radio_cd53_ident.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
OTHER_FILES +=
