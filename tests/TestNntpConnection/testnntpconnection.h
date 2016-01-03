#ifndef TESTNNTPCONNECTION_H
#define TESTNNTPCONNECTION_H

#include <QtTest/QtTest>

#include "../../nntpproxy.h"
#include "../../nntpconnection.h"


class TestNntpConnection : public QObject
{
    Q_OBJECT
public:
    explicit TestNntpConnection();

signals:
    void startConnection(const char* aHost, ushort aPort);

public slots:
    void inputAuthenticated();

private slots:
    void initTestCase(); // Called once before the test cases

    void test_authenticate();
    void test_authenticate_ssl();


private:
    NntpServerParameters *iParams;
    NntpServer           *iServer;
    NntpConnection       *iNntpCon;
};

#endif // TESTNNTPCONNECTION_H
