#include "testnntpconnection.h"
#include "../../constants_tests.h"


TestNntpConnection::TestNntpConnection():
    iParams(Q_NULLPTR), iServer(Q_NULLPTR), iNntpCon(Q_NULLPTR)
{}

void TestNntpConnection::initTestCase(){
    bool init = NntpProxy::initStatics();
    std::cout << "Proxy init? " << init << "\n";
}


void TestNntpConnection::inputAuthenticated(){

    std::cout << "Authenticated!!! \n";
}

/*
void TestNntpConnection::handleSocketError(QString aError){
    QString str("Error socket: ");
    str += aError;
    log(str);

    closeConnection();
}

void TestNntpConnection::closeConnection(){
    log("NntpConnection closed...");

}
*/

void TestNntpConnection::test_authenticate(){
    iParams = new NntpServerParameters(cTestNntpServParam());
    iServer = new NntpServer(*iParams);

    iNntpCon = new NntpConnection(0, *iServer);
    connect(this, &TestNntpConnection::startConnection, iNntpCon, &Connection::startTcpConnection);
//    connect(iNntpCon, &NntpConnection::closed, this, &TestNntpConnection::closeConnection);
//    connect(iNntpCon, &Connection::socketError, this, &TestNntpConnection::handleSocketError);
    connect(iNntpCon, &NntpConnection::authenticated, this, &TestNntpConnection::inputAuthenticated);



    emit startConnection(iParams->name.toLatin1().constData(), iParams->port);

    QVERIFY(iNntpCon->doAuthentication());

    delete iNntpCon;
    delete iServer;
    delete iParams;
}


void TestNntpConnection::test_authenticate_ssl(){
    iParams = new NntpServerParameters(cTestNntpServParamSSL());
    iServer = new NntpServer(*iParams);

    iNntpCon = new NntpConnection(0, *iServer);
    connect(this, &TestNntpConnection::startConnection, iNntpCon, &Connection::startTcpConnection);
//    connect(iNntpCon, &NntpConnection::closed, this, &TestNntpConnection::closeConnection);
//    connect(iNntpCon, &Connection::socketError, this, &TestNntpConnection::handleSocketError);
    connect(iNntpCon, &NntpConnection::authenticated, this, &TestNntpConnection::inputAuthenticated);



    emit startConnection(iParams->name.toLatin1().constData(), iParams->port);

    QVERIFY(iNntpCon->doAuthentication());

    delete iNntpCon;
    delete iServer;
    delete iParams;
}
