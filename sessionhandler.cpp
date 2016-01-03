#include "sessionhandler.h"
#include "inputconnection.h"
#include "sessionmanager.h"
#include "nntpproxy.h"
#include "mythread.h"
#include "nntpconnection.h"
#include "user.h"
#include "database.h"


#include <QTextStream>

SessionHandler::SessionHandler(qintptr aSocketDescriptor, SessionManager & aInputMgr,
                               MyThread *aThread):
    QObject(), iSocketDescriptor(aSocketDescriptor),
    iInputCon(Q_NULLPTR), iSessionMgr(aInputMgr),
    iThread(aThread), iNntpCon(Q_NULLPTR), iUser(Q_NULLPTR),
    isActive(true),
    iLogPrefix(QString("SessionHandler").append("[").append(QString::number(iSocketDescriptor)).append("] ")),
    isForwarding(false),
    isNntpServerActive(true),
    mNntpConOffered(Q_NULLPTR), wNntpConOffered(Q_NULLPTR), isNntpConOffered(false),
    mShutdownManager(Q_NULLPTR), wShutdownManager(Q_NULLPTR), isShutdownManager(false)
{
#ifdef LOG_CONSTRUCTORS
    _log("Constructor");
#endif

    iInputCon = new InputConnection(iSocketDescriptor);
    iInputCon->moveToThread(iThread);

    connect(this, &SessionHandler::startConnection, iInputCon, &Connection::startTcpConnection);
    connect(this, &SessionHandler::stopSession, this, &SessionHandler::closeSession);

    connect(iInputCon, &InputConnection::closed, this, &SessionHandler::closeSession);
    connect(iInputCon, &Connection::socketError, this, &SessionHandler::handleSocketError);
    connect(iInputCon, &InputConnection::authenticated, this, &SessionHandler::inputAuthenticated);

    connect(this, &SessionHandler::deleteSession, this, &QObject::deleteLater);
    qRegisterMetaType<std::string>("std::string" );
}



void SessionHandler::inputAuthenticated(std::string aLogin, std::string aPass){
    QString tmp("TCP authentication done, user: ");
    tmp += aLogin.c_str();

    _log(tmp);

    if (!NntpProxy::encrypt(aPass)){
        _log("Error encrypting pass..");
        iInputCon->write(Nntp::getResponse(502));
        closeSession();
        return;
    }

    iUser = iSessionMgr.getUser(iInputCon->getIpAddress(), QString::fromStdString(aLogin));

    if (!iSessionMgr.checkUserAuthentication(iUser, QString::fromStdString(aPass)) ){
        _log("Error Db Authentication...");
        iInputCon->write(Nntp::getResponse(502));
        closeSession();
        return;
    }

    if (iUser->getNumberOfInputConnection() > NntpProxy::getMaxConnectionsPerUser()){
        _log("Error: User has already the max number of connection...");
        iInputCon->write(Nntp::getResponse(502));
        closeSession();
        return;
    }

    startForwarding();
}


void SessionHandler::startForwarding(){

    iNntpCon = iSessionMgr.getNntpConnection(iInputCon->getId(), iUser);


    if (iNntpCon == Q_NULLPTR){
        _log("Error couldn't get an NNTP connection");

        // check if we can steal one to another user
        iNntpCon = iSessionMgr.tryToGetNntpConnectionFromOtherUser(iInputCon->getId(), iUser);

        if (iNntpCon == Q_NULLPTR){
            _log("Couldn't get a connection from another user...");
            iInputCon->write(Nntp::getResponse(502));
            closeSession();
            return;
        }
    }


//    iNntpCon->moveToThread(iThread);
    connect(iNntpCon, &NntpConnection::closed, this, &SessionHandler::closeNntpConnection);
    connect(iNntpCon, &Connection::socketError, this, &SessionHandler::handleNntpSocketError);
    connect(iNntpCon, &NntpConnection::serverRemoved, this, &SessionHandler::nntpServerRemoved);




    if (!iNntpCon->startTcpConnection(
                iNntpCon->getServerHost().toStdString().c_str(),
                iNntpCon->getServerPort())){
        _log("Error stating Nntp Connection...");
        iInputCon->write(Nntp::getResponse(502));
        closeSession();
        return;
    }

    if (!iNntpCon->doAuthentication()){
        _log("Error Nntp Authentication...");
        iInputCon->write(Nntp::getResponse(502));
        closeSession();
        return;
    }

    iInputCon->write(Nntp::getResponse(281));


    iUser->newNntpConnection(iNntpCon->getServerId());

    QTextStream &ostream = NntpProxy::acquireLog(iLogPrefix);
    ostream << "User is ready to use the NntpConnection. " << *iUser;
    NntpProxy::releaseLog();

    iInputCon->setOutput(iNntpCon);
    iNntpCon->setOutput(iInputCon);


    iNntpCon->startAsyncRead();

    // wait for input cmds
    iInputCon->startAsyncRead();

    isForwarding = true;

}

void SessionHandler::nntpServerRemoved(){
    _log("NntpServer got removed from the system...");
    isNntpServerActive = false;
    closeSession();
}

void SessionHandler::closeNntpConnection(){
    _log("closeNntpConnection");
    closeSession();
}

NntpConnection *SessionHandler::offerNntpConnection(){
    isForwarding = false;
    iUser->addDownloadSize(iNntpCon->getDownloadSize());
    iUser->delNntpConnection(iNntpCon->getServerId());
    return iNntpCon;
}

void SessionHandler::waitNntpSessionClosed(){
    isNntpConOffered = true;

    mNntpConOffered = new QMutex();
    wNntpConOffered = new QWaitCondition();
    mNntpConOffered->lock();
    wNntpConOffered->wait(mNntpConOffered);
    mNntpConOffered->unlock();
}

void SessionHandler::waitForDeletion(){
    isShutdownManager = true;

    mShutdownManager = new QMutex();
    wShutdownManager = new QWaitCondition();
    mShutdownManager->lock();
    wShutdownManager->wait(mShutdownManager);
    mShutdownManager->unlock();
}

SessionHandler::~SessionHandler(){
#ifdef LOG_CONSTRUCTORS
    _log("Destructor");
#endif
    if (isForwarding){
        // Add the download size of this connection to the user
        // (it may have several connections)
        iUser->addDownloadSize(iNntpCon->getDownloadSize());
        iUser->delNntpConnection(iNntpCon->getServerId());

        if (isNntpServerActive)
            iSessionMgr.releaseNntpConnection(iNntpCon);
    }

    delete iNntpCon;
    iNntpCon  = Q_NULLPTR;

    if (isNntpConOffered){
        wNntpConOffered->wakeOne();
        mNntpConOffered->lock();
        mNntpConOffered->unlock();

        delete wNntpConOffered;
        delete mNntpConOffered;
    }

    delete iInputCon;
    iInputCon = Q_NULLPTR;

    if (iUser)
        iSessionMgr.releaseUser(iUser); // we don't own iUser, don't delete it!!!

    if (isShutdownManager){
        wShutdownManager->wakeOne();
        mShutdownManager->lock();
        mShutdownManager->unlock();

        delete wShutdownManager;
        delete mShutdownManager;
    }

    emit destroyed();
}


void SessionHandler::handleSocketError(QString aError){
    QString str("Error socket: ");
    str += aError;
    _log(str);
    closeSession();
}

void SessionHandler::handleNntpSocketError(QString aError){
    QString str("Error Nntp socket: ");
    str += aError;
    _log(str);
    closeSession();
}

void SessionHandler::closeSession(){
    if (isActive){ // avoid closing several times
        _log("closeSession SessionHandler");
        isActive = false;
        iInputCon->closeConnection();
        iSessionMgr.erase(this, !isNntpConOffered);
        emit deleteSession();
    }
}

QTextStream &  operator<<(QTextStream & stream, const SessionHandler &aSession){
    stream << aSession.iLogPrefix;
    return stream;
}
