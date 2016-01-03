#include "testdatabase.h"
#include "../../constants_tests.h"
#include "../../user.h"


void TestDatabase::initTestCase(){
    bool init = NntpProxy::initStatics();
    std::cout << "Proxy init? " << init << "\n";
}

void TestDatabase::cleanupTestCase(){

}

void TestDatabase::init(){
    iDb = new Database();
    DatabaseParameters dbParam(cTestDbParams());
    iDb->addDatabase(&dbParam);
}

void TestDatabase::cleanup(){
    delete iDb;
}


void TestDatabase::test_connect(){
    QVERIFY(iDb->connect());
}

void TestDatabase::test_checkAuthentication(){
    User user_ok("127.0.0.1", cGoodUserNntp);
    QVERIFY(iDb->checkAuthentication(&user_ok, cGoodPassNntp));

    User user_bad("127.0.0.1", "nobody");
    QVERIFY(!iDb->checkAuthentication(&user_bad, cGoodPassNntp));
}

void TestDatabase::test_addUserSize(){
    User user_ok("127.0.0.1", cGoodUserNntp);
    QVERIFY(iDb->checkAuthentication(&user_ok, cGoodPassNntp));

    user_ok.addDownloadSize(500);
    QVERIFY(iDb->addUserSize(&user_ok) > 0);
}
