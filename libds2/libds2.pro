#-------------------------------------------------
#
# Project created by QtCreator 2014-08-24T14:28:48
#
#-------------------------------------------------

QT -= gui
QT += sql

TARGET = ds2
TEMPLATE = lib

DEFINES += LIBDS2_LIBRARY

SOURCES += \
           ds2packet.cpp \
           controlunit.cpp \
           operation.cpp \
           result.cpp \
           manager.cpp \
    dpp_v1_parser.cpp

HEADERS +=\
           ds2packet.h \
           operation.h \
           result.h \
           controlunit.h \
           manager.h \
    dpp_v1_parser.h

unix {
    target.path = /usr/lib
    INSTALLS += target
    VERSION = 0.5
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
