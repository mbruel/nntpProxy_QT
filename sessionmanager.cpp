#include "sessionmanager.h"
#include "sessionhandler.h"
#include "mythread.h"
#include "user.h"
#include "nntpconnection.h"

SessionManager::SessionManager(UserManager & aUserMgr, Database & aDb, NntpServerManager & aSrvMgr) :
    MyManager<SessionHandler>("Session"), iUserMgr(aUserMgr), iDb(aDb), iSrvMgr(aSrvMgr)
{}

SessionManager::~SessionManager(){
#ifdef LOG_CONSTRUCTORS
    QString str("Destructor, ");
    str += getSizeStr_noLock();
    _log(str);
#endif

    while (!iList.isEmpty()){
        SessionHandler * session = iList.front();
        QString str("Send signal to close session #");
        str += QString::number(session->getId());
        _log(str);

        emit session->stopSession();
        session->waitForDeletion();
        _log("session deleted...");
    }

    _log("All session handlers are properly closed!");
}


SessionHandler* SessionManager::newSession(qintptr aSocketDescriptor, MyThread *aThread){
    QMutexLocker lock(mMutex);
    SessionHandler *session = new SessionHandler(aSocketDescriptor, *this, aThread);

    if (session){
        iList.append(session);

        QTextStream &is = NntpProxy::acquireLog(iLogPrefix);
        is << "New " << iDataName << " with id: " << aSocketDescriptor
           << " added.  " << getSizeStr_noLock();
        NntpProxy::releaseLog();

    } else {
        NntpProxy::log(iLogPrefix, "Error creating session...");
    }

    return session;
}

NntpConnection * SessionManager::tryToGetNntpConnectionFromOtherUser(qintptr aInputConId, User *aUser){
    _log("[tryToGetNntpConnectionFromOtherUser] >>>>>");

    QMutexLocker lockUserMgr(iUserMgr.mMutex); // it's a friend
    QMutexLocker lockNntpServMgr(iSrvMgr.mMutex); // also a friend
    QMutexLocker lock(mMutex);

    ushort averageNntpConsPerUser = iSrvMgr.getMaxNumberOfConnections() / iUserMgr.size_noLock();

    iUserMgr.lockUser(aUser);

    // Check if the user has already more connection than the average
    if (iUserMgr.getUserNumberOfNntpCons_noLock(aUser) >= averageNntpConsPerUser){
        QString str("The User already have the average number of connection: ");
        str += QString::number(averageNntpConsPerUser);
        _log(str);
        iUserMgr.unlockUser(aUser);
        return Q_NULLPTR;
    }

    // Get a user having more connections than the average
    User *victim=iUserMgr.getUserHavingTooMuchNntpConnection_noLock(averageNntpConsPerUser);
    if (victim == Q_NULLPTR){
        QString str("There are no user with more connection than the average: ");
        str += QString::number(averageNntpConsPerUser);
        _log(str);
        iUserMgr.unlockUser(aUser);
        return Q_NULLPTR;
    }


    // Find the first Session of the victim
    NntpConnection *con = Q_NULLPTR;
    for (int i=0; i<iList.size(); ++i){
        SessionHandler *session = iList[i];
        User *user = session->iUser;
        if (user == victim){
            QString str("We found a victim, session: ");
            str += QString::number(session->getId());
            str += ". user: ";
            str += user->str();
            _log(str);

            con           = session->offerNntpConnection();
            ushort servId = con->getServerId();
            if (!iSrvMgr.releaseNntpConnection(con, false)){
                _log("Error releasing Nntp Connection");
                iUserMgr.unlockUser(aUser);
                return Q_NULLPTR;
            }

            // send a signal to close the session
            emit session->stopSession();
            _log("emit session->stopSession (switch to session thread)\n");

            // wait for the NntpConnection to have been released
            session->waitNntpSessionClosed();

            _log("\nwaitNntpSessionClosed ok");

            con = iSrvMgr.getOfferedNntpConnectionFromServer_noLock(aInputConId, servId);
            break;
        }
    }


    iUserMgr.unlockUser(aUser);
    _log("[tryToGetNntpConnectionFromOtherUser] <<<<<\n\n");
    return con;
}
