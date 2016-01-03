#ifndef TESTUSER_H
#define TESTUSER_H

#include <QtTest/QtTest>

#include "../../user.h"
#include <QTextStream>

#undef LOG_CONSTRUCTORS

class TestUser: public QObject
{
    Q_OBJECT

public:
    TestUser():iUser(Q_NULLPTR), cout(stdout) {}

private slots:
    void str();
    void downSize();
    void inputConnections();
    void nntpConnections();
    void hasConnectionWithServer();
    void getSetOfNntpServerOrderedByNumberOfConnections();

    void init(); // called before each test case
    void cleanup(); // called after each test case


    void initTestCase(); // Called once before the test cases
//    void cleanupTestCase(); // Called once after all test cases

private:
    User *iUser;
    QTextStream cout;

    void printUser(){cout << *iUser << "\n";}
    void printSize(){cout << iUser->downSize() << "\n";}
};

#endif // TESTUSER_H
