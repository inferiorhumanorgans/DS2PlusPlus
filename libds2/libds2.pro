#-------------------------------------------------
#
# Project created by QtCreator 2014-08-24T14:28:48
#
#-------------------------------------------------

VERSION = 0.6.2

QT -= gui
QT += sql

CONFIG   += c++11

TARGET = ds2
TEMPLATE = lib

DEFINES += LIBDS2_LIBRARY LIBDS2_VERSION=\\\"$$VERSION\\\"

SOURCES += \
           ds2packet.cpp \
           controlunit.cpp \
           operation.cpp \
           result.cpp \
           manager.cpp \
           dpp_v1_parser.cpp \
           basepacket.cpp \
           kwppacket.cpp

HEADERS +=\
           ds2/ds2packet.h \
           ds2/operation.h \
           ds2/result.h \
           ds2/controlunit.h \
           ds2/manager.h \
           ds2/dpp_v1_parser.h \
           ds2/exceptions.h \
           ds2/basepacket.h \
           ds2/kwppacket.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../jsoncpp/ -ljsoncpp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../jsoncpp/ -ljsoncppd
else:unix: LIBS += -L$$OUT_PWD/../jsoncpp/ -ljsoncpp

INCLUDEPATH += $$PWD/../jsoncpp
DEPENDPATH += $$PWD/../jsoncpp

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../jsoncpp/libjsoncpp.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../jsoncpp/libjsoncppd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../jsoncpp/jsoncpp.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../jsoncpp/jsoncppd.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../jsoncpp/libjsoncpp.a

target.path = /Users/alex/ds2-install/lib/

headers.files = $$HEADERS
headers.path = /Users/alex/ds2-install/include/ds2/
INSTALLS += headers
