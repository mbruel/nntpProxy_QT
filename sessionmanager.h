#ifndef SessionManager_H
#define SessionManager_H

#include "constants.h"
#include "mymanager.h"
#include "usermanager.h"
#include "database.h"
#include "nntpservermanager.h"


QT_FORWARD_DECLARE_CLASS(SessionHandler)
QT_FORWARD_DECLARE_CLASS(MyThread)

/*!
 * \brief Manager of SessionHandler (does NOT own them)
 * - keep a list of all active Sessions
 * - provide them an interface to UserManager so they can get a User
 * - provide them an interface to the Database
 * - provide them an interface to NntpServerManager so they can get a NntpConnection
 */
class SessionManager : public MyManager<SessionHandler>
{
public:
    //! Constructor with handles on UserManager, Database and NntpServerManager
    explicit SessionManager(UserManager & aUserMgr, Database & aDb, NntpServerManager & aSrvMgr);
    SessionManager(const SessionManager &)              = delete;
    SessionManager(const SessionManager &&)             = delete;
    SessionManager & operator=(const SessionManager &)  = delete;
    SessionManager & operator=(const SessionManager &&) = delete;

     ~SessionManager(); //!< emit &SessionHandler::stopSession and wait for all of them to close properly

    /*!
     * \brief create a new SessionHandler with its socket descriptor and its thread
     * \param aSocketDescriptor: socket descriptor created within the QTcpSocket
     * \param aThread: Thread where the SessionHandler will be moved
     * \return the new SessionHandler
     */
    SessionHandler * newSession(qintptr aSocketDescriptor, MyThread *aThread);

    inline User * getUser(const QString & aIpAddress, const QString & aLogin); //!< return new or existing User
    inline bool releaseUser(User *aUser); //!< release user (via UserManager)

    //! Interface to the Database to check the user authentication
    inline bool checkUserAuthentication(User *const aUser, const QString aPass);

    //! Interface to NntpServerManager to get a new NntpConnection for the given user
    inline NntpConnection *getNntpConnection(qintptr aInputConId, User *aUser);

    //! If no more available NntpConnection, check if we can steal one from another user
    NntpConnection * tryToGetNntpConnectionFromOtherUser(qintptr aInputConId, User *aUser);
    inline bool releaseNntpConnection(NntpConnection *aNntpCon); //!< interface to NntpServerManager to release a NntpConnction


private:
    UserManager       & iUserMgr; //!< Handle on UserManager
    Database          & iDb;      //!< Handle on Database
    NntpServerManager & iSrvMgr;  //!< Handle on NntpServerManager
};


bool SessionManager::checkUserAuthentication(User *const aUser, const QString aPass){
    return iDb.checkAuthentication(aUser, aPass);
}

User * SessionManager::getUser(const QString & aIpAddress, const QString & aLogin){
    return iUserMgr.addUser(aIpAddress, aLogin);
}

bool SessionManager::releaseUser(User *aUser){
    return iUserMgr.releaseUser(aUser, iDb);
}

NntpConnection *SessionManager::getNntpConnection(qintptr aInputConId, User *aUser){
    return iSrvMgr.getNntpConnection(aInputConId, aUser);
}

bool SessionManager::releaseNntpConnection(NntpConnection *aNntpCon){
    return iSrvMgr.releaseNntpConnection(aNntpCon);
}

#endif // SessionManager_H
