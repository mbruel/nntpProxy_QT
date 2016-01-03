QT += core network sql testlib
QT -= gui

TARGET = testUserManager
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11
QMAKE_CXXFLAGS += -Wno-write-strings

TEMPLATE = app

# Test coverage
QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

LIBS += \
    -lgcov

SOURCES += main.cpp \
    testusermanager.cpp \
    ../../connection.cpp \
    ../../inputconnection.cpp \
    ../../log.cpp \
    ../../mythread.cpp \
    ../../nntp.cpp \
    ../../nntpconnection.cpp \
    ../../nntpproxy.cpp \
    ../../nntpserver.cpp \
    ../../nntpservermanager.cpp \
    ../../sessionhandler.cpp \
    ../../sessionmanager.cpp \
    ../../user.cpp \
    ../../usermanager.cpp \
    ../../database.cpp \
    ../../mycrypt.cpp

HEADERS += \
    testusermanager.h \
    ../../connection.h \
    ../../constants.h \
    ../../constants_tests.h \
    ../../inputconnection.h \
    ../../log.h \
    ../../mymanager.h \
    ../../mythread.h \
    ../../nntp.h \
    ../../nntpconnection.h \
    ../../nntpproxy.h \
    ../../nntpserver.h \
    ../../nntpservermanager.h \
    ../../sessionhandler.h \
    ../../sessionmanager.h \
    ../../user.h \
    ../../usermanager.h \
    ../../database.h \
    ../../mycrypt.h

