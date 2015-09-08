#-------------------------------------------------
#
# Project created by QtCreator 2014-08-29T21:48:24
#
#-------------------------------------------------

QT       -= core gui
CONFIG   += c++11

TARGET = jsoncpp
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    json_reader.cpp \
    json_value.cpp \
    json_writer.cpp

HEADERS += \
    json/assertions.h \
    json/autolink.h \
    json/config.h \
    json/features.h \
    json/forwards.h \
    json/json.h \
    json/reader.h \
    json/value.h \
    json/version.h \
    json/writer.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
