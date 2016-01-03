#include "testnntpserver.h"
#include "../../constants_tests.h"

void TestNntpServer::initTestCase(){
    bool init = NntpProxy::initStatics();
    std::cout << "Proxy init? " << init << "\n";
}

void TestNntpServer::init(){
    iParams = new NntpServerParameters(cTestNntpServParamSSL());
    iServer = new NntpServer(*iParams);
}

void TestNntpServer::cleanup(){
    delete iServer;
    delete iParams;
}

void TestNntpServer::test_getNntpConnection(){
    ushort maxCon = iServer->getMaxNumberOfConnections();
    QVERIFY(iServer->getNumberOfConnectionsAvailable() == maxCon);
    for (int i = 1; i <= maxCon; ++i){
        QVERIFY(iServer->hasConnectionAvailable()==true);
        NntpConnection * con = iServer->getNntpConnection(i);
        QVERIFY(iServer->getNumberOfConnectionsInUse() == i);
        QVERIFY(iServer->getNumberOfConnectionsAvailable() == maxCon - i);
        QVERIFY(con != Q_NULLPTR);
        QVERIFY(con->getId() == i);
    }

    QVERIFY(iServer->hasConnectionAvailable()==false);
    NntpConnection * con = iServer->getNntpConnection(666);
    QVERIFY(iServer->getNumberOfConnectionsInUse() == maxCon);
    QVERIFY(con == Q_NULLPTR);
}

void TestNntpServer::test_getNntpConnection_noLock(){
    ushort maxCon = iServer->getMaxNumberOfConnections();

    iServer->mMutex.lock();
    QVERIFY(iServer->getNumberOfConnectionsAvailable_noLock() == maxCon);
    for (int i = 1; i <= maxCon; ++i){
        QVERIFY(iServer->hasConnectionAvailable_noLock()==true);
        NntpConnection * con = iServer->getNntpConnection_noLock(i);
        QVERIFY(iServer->getNumberOfConnectionsInUse_noLock() == i);
        QVERIFY(iServer->getNumberOfConnectionsAvailable_noLock() == maxCon - i);
        QVERIFY(con != Q_NULLPTR);
        QVERIFY(con->getId() == i);
    }

    QVERIFY(iServer->hasConnectionAvailable_noLock()==false);
    NntpConnection * con = iServer->getNntpConnection_noLock(666);
    QVERIFY(iServer->getNumberOfConnectionsInUse_noLock() == maxCon);
    QVERIFY(con == Q_NULLPTR);

    iServer->mMutex.unlock();
}

void TestNntpServer::test_releaseNntpConnection(){
    NntpServerParameters paramBad;
    NntpServer servBad(paramBad);
    NntpConnection *con666 = new NntpConnection(666, servBad);

    QVERIFY(iServer->getNumberOfConnectionsInUse() == 0);
    bool ret = iServer->releaseNntpConnection(con666);
    QVERIFY(ret == false);
    QVERIFY(iServer->getNumberOfConnectionsInUse() == 0);

    ushort maxCon = iServer->getMaxNumberOfConnections();
    NntpConnection * con;
    for (int i = 1; i <= maxCon; ++i){
        QVERIFY(iServer->hasConnectionAvailable()==true);
        con = iServer->getNntpConnection(i);
        QVERIFY(iServer->getNumberOfConnectionsInUse() == i);
        QVERIFY(con != Q_NULLPTR);
        QVERIFY(con->getId() == i);
    }

    QVERIFY(iServer->hasConnectionAvailable()==false);

    ret = iServer->releaseNntpConnection(con666);
    QVERIFY(ret == false);
    QVERIFY(iServer->getNumberOfConnectionsInUse() == maxCon);

    QVERIFY(iServer->hasConnectionAvailable()==false);

    ret = iServer->releaseNntpConnection(con);
    QVERIFY(ret == true);
    QVERIFY(iServer->getNumberOfConnectionsInUse() == maxCon-1);
    QVERIFY(iServer->hasConnectionAvailable()==true);

    delete con666;
    delete con;
}

void TestNntpServer::test_canUseAllConnections_ok(){
    QVERIFY(iServer->canUseAllConnections());
}

void TestNntpServer::test_canUseAllConnections_ko(){
    delete iServer;

    iParams->maxConnections = 50;
    iServer = new NntpServer(*iParams);

    QVERIFY(!iServer->canUseAllConnections());
}
