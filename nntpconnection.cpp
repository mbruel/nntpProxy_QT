#include "nntpconnection.h"
#include "nntp.h"
#include "nntpproxy.h"


NntpConnection::NntpConnection(qintptr aInputId,
                               const NntpServer & aServer):
    Connection(aInputId, aServer.isSsl(), false, "NntpConnection"),
    iServer(aServer), iDownloadSize(0)
{
    iLogPrefix.append("Serv[").append(QString::number(iServer.getId())).append("] ");
#ifdef LOG_CONSTRUCTORS
    _log("Constructor");
#endif
}


bool NntpConnection::doAuthentication(){
#ifdef LOG_NEWS_AUTH
    QString str("> doAuthentication user: ");
    str += iServer.getAuthUser();
    _log(str);
#endif

    std::string user = iServer.getAuthUser().toStdString();
    if (!NntpProxy::decrypt(user)){
        QString err("Error decrypting user login: ");
        err += iServer.getAuthUser();
        _log(err);
        emit socketError(err);
        return false;
    }

    std::string pass = iServer.getAuthPass().toStdString();
    if (!NntpProxy::decrypt(pass)){
        QString err("Error decrypting user password: ");
        err += iServer.getAuthPass();
        _log(err);
        emit socketError(err);
        return false;
    }

    std::string cmd(Nntp::AUTHINFO_USER);
    cmd += user;
    cmd += Nntp::ENDLINE;


    iSocket->write(cmd.c_str());

    do {
        iSocket->waitForReadyRead();
    } while (!iSocket->canReadLine());
    QByteArray lineArr = iSocket->readLine();
#ifdef LOG_NEWS_AUTH
    {
        QString str("Authinfo pass response: ");
        str += lineArr.constData();
        _log(str);
    }
#endif

    if(strncmp(lineArr.constData(), Nntp::getResponse(381), 2) != 0){
        QString err("Wrong Authentication: response from '");
        err += cmd.c_str();
        err += "' should start with 38... resp: ";
        err += lineArr.constData();
#ifdef LOG_CONNECTION_ERRORS_BEFORE_EMIT_SIGNALS
        _log(err);
#endif
        emit socketError(err);
        return false;
    }


    cmd = Nntp::AUTHINFO_PASS;
    cmd += pass;
    cmd += Nntp::ENDLINE;
    iSocket->write(cmd.c_str());

    do {
        iSocket->waitForReadyRead();
    } while (!iSocket->canReadLine());
    lineArr = iSocket->readLine();

#ifdef LOG_NEWS_AUTH
    {
        QString str("Authinfo pass response: ");
        str += lineArr.constData();
        _log(str);
    }
#endif

    if(strncmp(lineArr.constData(), Nntp::getResponse(281), 2) != 0){
        QString err("Wrong Authentication: response from '");
        err += Nntp::AUTHINFO_PASS;
        err += "' should start with 28... resp: ";
        err += lineArr.constData();
#ifdef LOG_CONNECTION_ERRORS_BEFORE_EMIT_SIGNALS
        _log(err);
#endif
        emit socketError(err);
        return false;
    }

    _log("> Authentication succeed");
    emit authenticated();
    return true;
}


void NntpConnection::readyRead()
{
    while (iSocket->canReadLine()){
        QByteArray line = iSocket->readLine();

#ifdef LOG_NEWS_DATA
        QString str("Data In: ");
        str += line;
        _log(str);
#endif

        if (iOutputCon){
            iOutputCon->write(line);
            iDownloadSize += line.size();
        } else {
            closeConnection();
        }
    }
}


void NntpConnection::closeConnection(){
    // Stop async read
    Connection::closeConnection();

    // close input socket
    if (iSocket && iSocket->isOpen())
        iSocket->disconnectFromHost();

    if (iOutputCon) {
        iOutputCon->setOutput(Q_NULLPTR); // To avoid circular calls
        iOutputCon->closeConnection();
        iOutputCon = Q_NULLPTR;
    }
}



void NntpConnection::disconnected(){
    _log("Disconnected...");
    emit closed();
}
