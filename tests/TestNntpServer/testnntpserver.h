#ifndef TESTNNTPSERVER_H
#define TESTNNTPSERVER_H


#include <QtTest/QtTest>

#include "../../nntpproxy.h"
#include "../../nntpconnection.h"


class TestNntpServer : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase(); // Called once before the test cases

    void init(); // called before each test case
    void cleanup(); // called after each test case

    void test_getNntpConnection();
    void test_getNntpConnection_noLock();
    void test_releaseNntpConnection();
    void test_canUseAllConnections_ok();
    void test_canUseAllConnections_ko();


private:
    NntpServerParameters *iParams;
    NntpServer           *iServer;
};


#endif // TESTNNTPSERVER_H
