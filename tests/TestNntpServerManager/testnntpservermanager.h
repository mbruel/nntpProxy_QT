#ifndef TESTNNTPSERVERMANAGER_H
#define TESTNNTPSERVERMANAGER_H

#include <QtTest/QtTest>

#include "../../nntpproxy.h"
#include "../../nntpservermanager.h"
#include "../../usermanager.h"
#include <QVector>

class TestNntpServerManager : public QObject
{
    Q_OBJECT
public:
    TestNntpServerManager();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void init(); // called before each test case
    void cleanup(); // called after each test case

    void test_add_remove_server();

    void test_canConnectToNntpServers();

    void test_getAllCons2serv1user();

    void test_stealingConnections();

private:
    void createUserUsingAllConnections(User *aUser);
    void deleteConnections();

private:
    NntpServerManager              *iSrvMgr;
    QVector<NntpServerParameters *> iServParams;
    UserManager                    *iUserMgr;
    QVector<NntpConnection *>       iNntpCons;
    Database                       *iDb;
};



#endif // TESTNNTPSERVERMANAGER_H
