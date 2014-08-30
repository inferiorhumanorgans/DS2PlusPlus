#-------------------------------------------------
#
# Project created by QtCreator 2014-08-24T14:41:40
#
#-------------------------------------------------

QT       -= gui
QT       += serialport testlib sql

TARGET = tst_ds2test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lds2
INCLUDEPATH += ../libds2
LIBPATH += ../libds2

SOURCES += \
    main.cpp \
    ihka46_ident.cpp \
    kombi46_ident.cpp \
    dme_ms420_ident.cpp \
    dme_ms420_status.cpp \
    zke5_ident.cpp \
    radio_cd53_ident.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

OTHER_FILES +=

HEADERS += \
    ihka46_ident.h \
    kombi46_ident.h \
    dme_ms420_ident.h \
    dme_ms420_status.h \
    zke5_ident.h \
    radio_cd53_ident.h
