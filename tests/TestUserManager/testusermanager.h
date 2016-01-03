#ifndef TESTUSERMANAGER_H
#define TESTUSERMANAGER_H


#include <QtTest/QtTest>

#include "../../usermanager.h"
#include "../../user.h"
#include "../../nntpproxy.h"
#include "../../database.h"

#include <QTextStream>

class TestUserManager: public QObject
{
    Q_OBJECT
public:
    explicit TestUserManager();

private slots:

    void init(); // called before each test case
    void cleanup(); // called after each test case

    void test_adduser();
    void test_releaseUser();

    void test_blockUser();

    void test_userWithTooMuchConnection();

    void initTestCase(); // Called once before the test cases
    void cleanupTestCase(); // Called once after all test cases

private:
    void printUser(User *aUser){cout << *aUser << "\n";}
    void printSize(User *aUser){cout << aUser->downSize() << "\n";}

private:
    UserManager *iUserMgr;
    QTextStream  cout;
    Database     *iDb;
};

#endif // TESTUSERMANAGER_H
