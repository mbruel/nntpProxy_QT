QT += core network sql
QT -= gui

TARGET = nntpProxyQT
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

QMAKE_CXXFLAGS += -Wno-write-strings

TEMPLATE = app

SOURCES += main.cpp \
    nntpproxy.cpp \
    inputconnection.cpp \
    connection.cpp \
    log.cpp \
    nntp.cpp \
    user.cpp \
    usermanager.cpp \
    nntpserver.cpp \
    nntpconnection.cpp \
    mythread.cpp \
    sessionhandler.cpp \
    sessionmanager.cpp \
    nntpservermanager.cpp \
    database.cpp \
    mycrypt.cpp

HEADERS += \
    nntpproxy.h \
    inputconnection.h \
    connection.h \
    log.h \
    nntp.h \
    constants.h \
    user.h \
    usermanager.h \
    nntpserver.h \
    nntpconnection.h \
    mythread.h \
    constants_tests.h \
    sessionhandler.h \
    sessionmanager.h \
    nntpservermanager.h \
    mymanager.h \
    database.h \
    mycrypt.h

