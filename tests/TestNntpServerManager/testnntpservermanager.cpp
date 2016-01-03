#include "testnntpservermanager.h"
#include "../../constants_tests.h"

#include "../../nntpconnection.h"
#include "../../user.h"
#include "../../database.h"



TestNntpServerManager::TestNntpServerManager():
    iSrvMgr(Q_NULLPTR), iServParams(), iNntpCons(), iDb(Q_NULLPTR)
{}

void TestNntpServerManager::initTestCase(){
    bool init = NntpProxy::initStatics();
    std::cout << "Proxy init? " << init << "\n";

    iServParams.append(new NntpServerParameters(cTestNntpServParam()));
    iUserMgr = new UserManager();
    iDb = new Database();
    DatabaseParameters dbParam(cTestDbParams());
    iDb->addDatabase(&dbParam);
}

void TestNntpServerManager::cleanupTestCase(){
    for (int i=0; i< iServParams.size(); ++i)
        delete iServParams[i];
    iServParams.clear();
    delete iUserMgr;
    delete iDb;
}

void TestNntpServerManager::init(){
    iSrvMgr = new NntpServerManager(iServParams, *iUserMgr);
}

void TestNntpServerManager::cleanup(){
    delete iSrvMgr;
}



void TestNntpServerManager::test_add_remove_server(){

    NntpProxy::log("[TestNntpServerManager] ", "test_add_remove_server");

    QVERIFY(iSrvMgr->size() == 1);

    NntpServerParameters * currentParam = iServParams[0];
    QVERIFY(iSrvMgr->getMaxNumberOfConnections()== currentParam->maxConnections);
    QVERIFY(iSrvMgr->getNumberOfConnectionsAvailable()== currentParam->maxConnections);
    QVERIFY(iSrvMgr->getNumberOfConnectionsInUse() == 0);
    QVERIFY(iSrvMgr->getMaxNumberOfConnections(0)== currentParam->maxConnections);
    QVERIFY(iSrvMgr->getNumberOfConnectionsAvailable(0)== currentParam->maxConnections);
    QVERIFY(iSrvMgr->getNumberOfConnectionsInUse(0) == 0);

    NntpServerParameters newParam(cTestNntpServParamSSL());
    QVERIFY(iSrvMgr->addNntpServer(newParam) != -1);

    QVERIFY(iSrvMgr->size() == 2);
    QVERIFY(iSrvMgr->getMaxNumberOfConnections(1)== newParam.maxConnections);
    QVERIFY(iSrvMgr->getNumberOfConnectionsAvailable(1)== newParam.maxConnections);
    QVERIFY(iSrvMgr->getNumberOfConnectionsInUse(1) == 0);

    ushort maxCons = currentParam->maxConnections + newParam.maxConnections;
    QVERIFY(iSrvMgr->getMaxNumberOfConnections() == maxCons);
    QVERIFY(iSrvMgr->getNumberOfConnectionsAvailable() == maxCons);
    QVERIFY(iSrvMgr->getNumberOfConnectionsInUse() == 0);

    // try removing non existing serv
    QVERIFY(!iSrvMgr->removeNntpServer(666));

    // Remove first server
    QVERIFY(iSrvMgr->removeNntpServer(0));
    QVERIFY(iSrvMgr->size() == 1);
    QVERIFY(iSrvMgr->getMaxNumberOfConnections() == newParam.maxConnections);
    QVERIFY(iSrvMgr->getNumberOfConnectionsAvailable() == newParam.maxConnections);
    QVERIFY(iSrvMgr->getNumberOfConnectionsInUse() == 0);
}



void TestNntpServerManager::test_canConnectToNntpServers(){
    NntpProxy::log("[TestNntpServerManager] ", "test_canConnectToNntpServers");

    QVERIFY(iSrvMgr->canConnectToNntpServers());

    NntpServerParameters wrongServ("news.wrongHost.com", 119, false, "", "", 3);
    QVERIFY(iSrvMgr->addNntpServer(wrongServ) == -1);
}



void TestNntpServerManager::createUserUsingAllConnections(User *aUser){
    NntpProxy::log("[TestNntpServerManager] ", "test_getAllCons2serv1user");

    NntpServerParameters newParam(cTestNntpServParamSSL());
    short idServ2 = iSrvMgr->addNntpServer(newParam);
    short idServ1 = idServ2 -1;
    QVERIFY(idServ2 != -1);

    QVERIFY(iSrvMgr->size() == 2);
    ushort maxCons = iSrvMgr->getMaxNumberOfConnections();


    // Static settings in constants_test.h

    short nb = iSrvMgr->getMaxNumberOfConnections(idServ1);
    QVERIFY(iSrvMgr->getMaxNumberOfConnections(idServ1) == 2);
    QVERIFY(iSrvMgr->getMaxNumberOfConnections(idServ2) == 3);
    QVERIFY(maxCons  == 5);


    NntpConnection * con1 = iSrvMgr->getNntpConnection(1, aUser);
    QVERIFY(con1->getServerId() == idServ2); // serv2 has more connections
    aUser->newNntpConnection(idServ2);
    QVERIFY(iSrvMgr->getNumberOfConnectionsAvailable(idServ2) == 2);
    QVERIFY(iSrvMgr->getNumberOfConnectionsInUse(idServ2)     == 1);
    QVERIFY(iSrvMgr->getNumberOfConnectionsAvailable()  == maxCons - 1);
    QVERIFY(iSrvMgr->getNumberOfConnectionsInUse()      == 1);



    NntpConnection * con2 = iSrvMgr->getNntpConnection(2, aUser);
    QVERIFY(con2->getServerId() == idServ1); // we don't have any connection from serv1
    aUser->newNntpConnection(idServ1);
    QVERIFY(iSrvMgr->getNumberOfConnectionsAvailable(idServ1) == 1);
    QVERIFY(iSrvMgr->getNumberOfConnectionsInUse(idServ1)     == 1);
    QVERIFY(iSrvMgr->getNumberOfConnectionsAvailable()  == maxCons - 2);
    QVERIFY(iSrvMgr->getNumberOfConnectionsInUse()      == 2);


    NntpConnection * con3 = iSrvMgr->getNntpConnection(3, aUser);
    QVERIFY(con3->getServerId() == idServ1); // serv1 has still some connections
    aUser->newNntpConnection(idServ1);
    QVERIFY(iSrvMgr->getNumberOfConnectionsAvailable(idServ1) == 0);
    QVERIFY(iSrvMgr->getNumberOfConnectionsInUse(idServ1)     == 2);
    QVERIFY(iSrvMgr->getNumberOfConnectionsAvailable()  == maxCons - 3);
    QVERIFY(iSrvMgr->getNumberOfConnectionsInUse()      == 3);


    NntpConnection * con4 = iSrvMgr->getNntpConnection(4, aUser);
    QVERIFY(con4->getServerId() == idServ2); // less connection from serv2
    aUser->newNntpConnection(idServ2);
    QVERIFY(iSrvMgr->getNumberOfConnectionsAvailable(idServ2) == 1);
    QVERIFY(iSrvMgr->getNumberOfConnectionsInUse(idServ2)     == 2);
    QVERIFY(iSrvMgr->getNumberOfConnectionsAvailable()  == maxCons - 4);
    QVERIFY(iSrvMgr->getNumberOfConnectionsInUse()      == 4);



    NntpConnection * con5 = iSrvMgr->getNntpConnection(5, aUser);
    QVERIFY(con5->getServerId() == idServ2); // serv2 has still some connections
    aUser->newNntpConnection(idServ2);
    QVERIFY(iSrvMgr->getNumberOfConnectionsAvailable(idServ2) == 0);
    QVERIFY(iSrvMgr->getNumberOfConnectionsInUse(idServ2)     == 3);
    QVERIFY(iSrvMgr->getNumberOfConnectionsAvailable()  == maxCons - 5);
    QVERIFY(iSrvMgr->getNumberOfConnectionsInUse()      == 5);

    NntpProxy::log("[TestNntpServerManager] ", aUser->str());
    QCOMPARE(aUser->str(),
             QString("User #0: mb@127.0.0.1 (in: 1, out: 5) {serv: %1. cons: 2} {serv: %2. cons: 3} ").arg(
                 QString::number(idServ1), QString::number(idServ2)));

    iNntpCons.append(con1);
    iNntpCons.append(con2);
    iNntpCons.append(con3);
    iNntpCons.append(con4);
    iNntpCons.append(con5);
}

void TestNntpServerManager::deleteConnections(){
    for (int i=0; i<iNntpCons.size(); ++i)
        QVERIFY(iSrvMgr->releaseNntpConnection(iNntpCons[i]));

    for (int i=0; i<iNntpCons.size(); ++i)
        delete iNntpCons[i];

    iNntpCons.clear();
}

void TestNntpServerManager::test_getAllCons2serv1user(){
    User *user = iUserMgr->addUser("127.0.0.1", "mb");

    createUserUsingAllConnections(user);

    NntpConnection * con666 = iSrvMgr->getNntpConnection(666, user);
    QVERIFY(con666 == Q_NULLPTR);

    deleteConnections();
    QVERIFY(iUserMgr->releaseUser(user, *iDb));
}


void TestNntpServerManager::test_stealingConnections(){
    User *user = iUserMgr->addUser("127.0.0.1", "mb");
    createUserUsingAllConnections(user);


    User *user2 = iUserMgr->addUser("127.0.0.2", "bob");





    deleteConnections();
    QVERIFY(iUserMgr->releaseUser(user, *iDb));
    QVERIFY(iUserMgr->releaseUser(user2, *iDb));

}
