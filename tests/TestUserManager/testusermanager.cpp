#include "testusermanager.h"
#include "../../constants_tests.h"

TestUserManager::TestUserManager(): iUserMgr(Q_NULLPTR), cout(stdout){}


void TestUserManager::initTestCase(){
    bool init = NntpProxy::initStatics();
    std::cout << "Proxy init? " << init << "\n";
    iDb = new Database();
    DatabaseParameters dbParam(cTestDbParams());
    iDb->addDatabase(&dbParam);
}

void TestUserManager::cleanupTestCase(){
    delete iDb;
}

void TestUserManager::init(){
    iUserMgr = new UserManager();
}


void TestUserManager::cleanup(){
    delete iUserMgr;
}

void TestUserManager::test_adduser()
{
    // Checking empty list
    QVERIFY(iUserMgr->size() == 0);

    // Adding first user
    User * user1 = iUserMgr->addUser("127.0.0.1", "mb");
    QVERIFY(iUserMgr->size() == 1);
//    printUser(user1);
    QCOMPARE(user1->str(), QString("User #0: mb@127.0.0.1 (in: 1, out: 0) "));


    // Same user, another connection
    User * user2 = iUserMgr->addUser("127.0.0.1", "mb");
//    printSize();
    QVERIFY(iUserMgr->size() == 1);
//    printUser(user2);
    QVERIFY(*user1 == *user2);
    QCOMPARE(user1->str(), QString("User #0: mb@127.0.0.1 (in: 2, out: 0) "));
//    printUser(user2);


    // Same user, another connection
    User * user3 = iUserMgr->addUser("127.0.0.1", "mb");
    QVERIFY(iUserMgr->size() == 1);
    QVERIFY(*user1 == *user3);
    QVERIFY(*user2 == *user3);
    QCOMPARE(user1->str(), QString("User #0: mb@127.0.0.1 (in: 3, out: 0) "));


    // Same user, different IP => New user
    User * user4 = iUserMgr->addUser("127.0.0.254", "mb");
    QVERIFY(iUserMgr->size() == 2);
//    printUser(user1);
    QCOMPARE(user4->str(), QString("User #0: mb@127.0.0.254 (in: 1, out: 0) "));
    QVERIFY(!(*user1 == *user4));


    // Different User
    User * user5 = iUserMgr->addUser("127.0.0.254", "john");
    QVERIFY(iUserMgr->size() == 3);
//    printUser(user1);
    QCOMPARE(user5->str(), QString("User #0: john@127.0.0.254 (in: 1, out: 0) "));
    QVERIFY(!(*user1 == *user5));
}


void TestUserManager::test_blockUser(){
    // Checking empty list
    QVERIFY(iUserMgr->size() == 0);
    QVERIFY(!iUserMgr->setUserBlocked("mb", true));

    // Adding first user
    User * user1 = iUserMgr->addUser("127.0.0.1", "mb");
    QVERIFY(iUserMgr->size() == 1);
//    printUser(user1);
    QCOMPARE(user1->str(), QString("User #0: mb@127.0.0.1 (in: 1, out: 0) "));

    QVERIFY(iUserMgr->setUserBlocked("mb", true));
    QVERIFY(!iUserMgr->setUserBlocked("wrongUser", false));
}


void TestUserManager::test_userWithTooMuchConnection(){
    // Add user john with 1 Nntp connection
    User * john = iUserMgr->addUser("127.0.0.254", "john");
    john->newNntpConnection(0);
    QCOMPARE(john->str(), QString("User #0: john@127.0.0.254 (in: 1, out: 1) {serv: 0. cons: 1} "));
    QVERIFY(iUserMgr->size() == 1);

    // Add user mb with 3 Nntp connections
    User * mb = iUserMgr->addUser("127.0.0.1", "mb");
    mb->newNntpConnection(0);
    mb->newNntpConnection(0);
    mb->newNntpConnection(0);
    QCOMPARE(mb->str(), QString("User #0: mb@127.0.0.1 (in: 1, out: 3) {serv: 0. cons: 3} "));
    QVERIFY(iUserMgr->size() == 2);

    // Add user bob with 2 Nntp connections
    User * bob = iUserMgr->addUser("127.0.0.254", "bob");
    bob->newNntpConnection(0);
    bob->newNntpConnection(0);
    QCOMPARE(bob->str(), QString("User #0: bob@127.0.0.254 (in: 1, out: 2) {serv: 0. cons: 2} "));
    QVERIFY(iUserMgr->size() == 3);

    User *userWithTooManyCon = iUserMgr->getUserHavingTooMuchNntpConnection_noLock(1);
    QVERIFY(userWithTooManyCon == mb);
}


void TestUserManager::test_releaseUser()
{
    QVERIFY(iUserMgr->size() == 0);

    // Test removing non existing user (shouldn't happen in real life...)
    User * wrongUser = new User("127.0.0.6", "bob");
    QVERIFY(!iUserMgr->releaseUser(wrongUser, *iDb));
    QVERIFY(iUserMgr->size() == 0);


    // Add user mb with 3 connections
    User * mb = iUserMgr->addUser("127.0.0.1", "mb");
    iUserMgr->addUser("127.0.0.1", "mb");
    iUserMgr->addUser("127.0.0.1", "mb");
    QCOMPARE(mb->str(), QString("User #0: mb@127.0.0.1 (in: 3, out: 0) "));
    QVERIFY(iUserMgr->size() == 1);

    // Add user john with 1 connection
    User * john = iUserMgr->addUser("127.0.0.254", "john");
    QCOMPARE(john->str(), QString("User #0: john@127.0.0.254 (in: 1, out: 0) "));
    QVERIFY(iUserMgr->size() == 2);


    iUserMgr->dump();

    // Delete mb connction
    QVERIFY(iUserMgr->releaseUser(mb, *iDb));
    QCOMPARE(mb->str(), QString("User #0: mb@127.0.0.1 (in: 2, out: 0) "));
    QVERIFY(iUserMgr->size() == 2);

    // Delete mb connction
    QVERIFY(iUserMgr->releaseUser(mb, *iDb));
    QCOMPARE(mb->str(), QString("User #0: mb@127.0.0.1 (in: 1, out: 0) "));
    QVERIFY(iUserMgr->size() == 2);


    iUserMgr->dump();

    // Release John
    QVERIFY(iUserMgr->releaseUser(john, *iDb));
    QVERIFY(iUserMgr->size() == 1);
    // John has no more input connection, so should have been deleted...
    john = Q_NULLPTR;


    // Release last mb connection
    QVERIFY(iUserMgr->releaseUser(mb, *iDb));
    QVERIFY(iUserMgr->size() == 0);
    mb = Q_NULLPTR;


    QVERIFY(!iUserMgr->releaseUser(wrongUser, *iDb));
    QVERIFY(iUserMgr->size() == 0);
    delete wrongUser;
}
