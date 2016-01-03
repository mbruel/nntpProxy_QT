#include "inputconnection.h"
#include "nntpproxy.h"


#include <regex>
#include <algorithm> //std::transform

InputConnection::InputConnection(qintptr aSocketDescriptor):
    Connection(aSocketDescriptor, NntpProxy::isClientSSL(), true, "InputConnection")
{
    connect(this, &InputConnection::connected, this, &InputConnection::doAuthentication);

#ifdef LOG_CONSTRUCTORS
    _log("Constructor");
#endif
}



bool InputConnection::doAuthentication(){

    _log("doAuthentication");

    const std::regex & authReg = Nntp::getCmdRegex(Nntp::CMDS::authinfo);
    std::smatch match;

    std::string user;
    std::string pass;
    bool isAuthenticated = false;

    for (int i=0; i < cAuthenticationTry; ++i){
        do {
            if (!iSocket->isOpen())
                return false;

            iSocket->waitForReadyRead();
        } while (iSocket->isOpen() && !iSocket->canReadLine());

        QByteArray lineArr = iSocket->readLine();

#ifdef LOG_INPUT_AUTH
        QString str("Data Authentication: ");
        str += lineArr.constData();
        _log(str);
#endif

        if(strcmp(lineArr.constData(), Nntp::QUIT) == 0){
            iSocket->disconnectFromHost();
            break;
        }

        std::string line(lineArr);
        std::regex_match(line, match, authReg);
        if (!match.size()){
            iSocket->write(Nntp::getResponse(480));
            continue;
        }

        std::string cmd = match[1];
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
        if (cmd == "user"){
            user = match[2];
            if (!pass.empty())
                isAuthenticated = true;
        } else {
            pass = match[2];
            if (!user.empty())
                isAuthenticated = true;
        }

        if (isAuthenticated) {
            break;
        } else {
            iSocket->write(Nntp::getResponse(381));
        }
    }

    if (isAuthenticated)
        emit authenticated(user, pass);
    else {
#ifdef LOG_CONNECTION_ERRORS_BEFORE_EMIT_SIGNALS
        _log("Wrong Authentication!");
#endif
        emit socketError("Wrong Authentication!");
    }

    return isAuthenticated;
}

void InputConnection::readyRead()
{
    if(iSocket->canReadLine()){
        QByteArray line = iSocket->readLine();

#ifdef LOG_INPUT_DATA
        QString str("Data In: ");
        str += line;
        _log(str);
#endif

        if(strcmp(line.constData(), Nntp::QUIT) == 0){
            closeConnection();
        } else {
            iOutputCon->write(line);
//            iSocket->write(line);
        }

    }
}


void InputConnection::closeConnection(){
    // Stop async read
    Connection::closeConnection();

    if (iOutputCon) {
        iOutputCon->setOutput(Q_NULLPTR); // To avoid circular calls
        iOutputCon->closeConnection();
        iOutputCon = Q_NULLPTR;
    }

    // close input socket
    if (iSocket && iSocket->isOpen())
        iSocket->disconnectFromHost();
}


// On disconnection we remove the connection from the SessionManager
// it will stop the thread and delete the connection
void InputConnection::disconnected()
{
    _log("> Disconnected");
    emit closed();
}



