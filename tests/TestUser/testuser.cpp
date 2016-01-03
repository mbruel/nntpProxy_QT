#include "testuser.h"
#include "../../nntpproxy.h"

void TestUser::initTestCase(){
    NntpProxy::initStatics();
}


void TestUser::init(){
    iUser = new User("127.0.0.1", "mb");

    // Adding connections
    iUser->newInputConnection();
    iUser->newNntpConnection(1);
    iUser->newInputConnection();

    iUser->newNntpConnection(2);
    iUser->newInputConnection();
    iUser->newNntpConnection(2);
    iUser->newInputConnection();
    iUser->newNntpConnection(2);

    iUser->newInputConnection();
    iUser->newNntpConnection(3);
    iUser->newInputConnection();
    iUser->newNntpConnection(3);
}


void TestUser::cleanup(){
    delete iUser;
}


void TestUser::str()
{
//    printUser();

    QString userStr = iUser->str();
    QCOMPARE(userStr, QString("User #0: mb@127.0.0.1 (in: 6, out: 6) {serv: 1. cons: 1} {serv: 2. cons: 3} {serv: 3. cons: 2} "));
}


void TestUser::downSize(){
    // Test downSize()
    ushort sec = 2;
    ushort debit = 720;

    iUser->addDownloadSize(debit*1024*sec); // 720 kB x 2s = 1.41MB
    QTest::qSleep(sec*1000);
//    printSize();

    QString downSize(iUser->downSize());
    QCOMPARE(downSize, QString("1.41 MB in 0:0:2 (720 kB/s)"));
}


void TestUser::inputConnections(){
    QVERIFY(iUser->getNumberOfInputConnection() == 6);

    iUser->newInputConnection();
    QVERIFY(iUser->getNumberOfInputConnection() == 7);

    for (int i=7; i>0; --i){
        QVERIFY(iUser->getNumberOfInputConnection() == i);
        iUser->delInputConnection();
    }
}


void TestUser::nntpConnections(){
    iUser->newNntpConnection(1);

    QCOMPARE(iUser->str(), QString("User #0: mb@127.0.0.1 (in: 6, out: 7) {serv: 1. cons: 2} {serv: 2. cons: 3} {serv: 3. cons: 2} "));

    QVERIFY(!iUser->delNntpConnection(666));

    QVERIFY(iUser->delNntpConnection(1));
    QVERIFY(iUser->delNntpConnection(1));
    QVERIFY(!iUser->delNntpConnection(1));

    QCOMPARE(iUser->str(), QString("User #0: mb@127.0.0.1 (in: 6, out: 5) {serv: 2. cons: 3} {serv: 3. cons: 2} "));

    QVERIFY(iUser->delNntpConnection(3));
    QVERIFY(iUser->delNntpConnection(3));
    QVERIFY(!iUser->delNntpConnection(3));

    QCOMPARE(iUser->str(), QString("User #0: mb@127.0.0.1 (in: 6, out: 3) {serv: 2. cons: 3} "));

}


void TestUser::hasConnectionWithServer(){
    iUser->lockNntpServList();
    QVERIFY(iUser->hasConnectionWithServer_noLock(1));
    QVERIFY(iUser->hasConnectionWithServer_noLock(2));
    QVERIFY(iUser->hasConnectionWithServer_noLock(3));
    QVERIFY(!iUser->hasConnectionWithServer_noLock(666));
    iUser->unlockNntpServList();
}


void TestUser::getSetOfNntpServerOrderedByNumberOfConnections()
{
    vectNntpSrvOrderByCons servers = iUser->getVectorOfNntpServerOrderedByNumberOfConnections_noLock();

    ushort index = 0;
    for (auto it = servers.cbegin(); it != servers.cend(); ++it){
//        std::cout << it->first << ": " << it->second << "\n";
        if (index == 0){
            QVERIFY(it->first  == 1);
            QVERIFY(it->second == 1);
        } else if (index == 1){
            QVERIFY(it->first  == 3);
            QVERIFY(it->second == 2);
        } else if (index == 2) {
            QVERIFY(it->first  == 2);
            QVERIFY(it->second == 3);
        }
        ++index;
    }
}
