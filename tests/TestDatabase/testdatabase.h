#ifndef TESTDATABASE_H
#define TESTDATABASE_H

#include <QtTest/QtTest>

#include "../../database.h"
#include "../../nntpproxy.h"

class TestDatabase : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase(); // Called once before the test cases
    void cleanupTestCase();  // Called once after all test cases

    void init(); // called before each test case
    void cleanup(); // called after each test case

    void test_connect();
    void test_checkAuthentication();
    void test_addUserSize();

 //   void test_releaseNntpConnection();

private:
    Database          *iDb;
};


#endif // TESTDATABASE_H
