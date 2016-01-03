QT += core network testlib sql
QT -= gui
QTPLUGIN += qsqlmysql


TARGET = testDatabase
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11
QMAKE_CXXFLAGS += -Wno-write-strings


# Test coverage
QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

LIBS += \
    -lgcov

TEMPLATE = app
SOURCES += main.cpp \
    testdatabase.cpp \
    ../../connection.cpp \
    ../../database.cpp \
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
    ../../mycrypt.cpp

HEADERS += \
    testdatabase.h \
    ../../connection.h \
    ../../constants.h \
    ../../constants_tests.h \
    ../../database.h \
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
    ../../mycrypt.h

