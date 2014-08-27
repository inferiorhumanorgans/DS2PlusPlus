#-------------------------------------------------
#
# Project created by QtCreator 2014-08-24T14:28:48
#
#-------------------------------------------------

QT -= gui
QT += serialport sql

TARGET = ds2
TEMPLATE = lib
CONFIG += serialport

DEFINES += LIBDS2_LIBRARY

SOURCES += \
           ds2packet.cpp \
           controlunit.cpp \
           operation.cpp \
           result.cpp \
           manager.cpp \
    exception.cpp

HEADERS +=\
           ds2packet.h \
           operation.h \
           result.h \
           controlunit.h \
           manager.h \
    exception.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
