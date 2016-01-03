#ifndef USERMANAGER_H
#define USERMANAGER_H

#include "constants.h"
#include "mymanager.h"
#include "user.h"

QT_FORWARD_DECLARE_CLASS(Database)
QT_FORWARD_DECLARE_CLASS(SessionManager)

/*!
 * \brief Manager of all the Users (own them)
 */
class UserManager : public MyManager<User>
{
public:
    friend class SessionManager;    //!< SessionManager::tryToGetNntpConnectionFromOtherUser is able to lock/unlock
    friend class NntpServerManager; //!< To be able to lock users when trying to get them a new connection
#ifdef TESTUSERMANAGER_H
    friend class TestUserManager;
#endif

    explicit UserManager(); //!< constructor
    UserManager(const UserManager &)              = delete;
    UserManager(const UserManager &&)             = delete;
    UserManager & operator=(const UserManager &)  = delete;
    UserManager & operator=(const UserManager &&) = delete;

    ~UserManager(); //!< destructor

    //! return an existing user or create a new one
    User * addUser(const QString & aIpAddress, const QString & aLogin);
    bool releaseUser(User *aUser, Database & aDb); //!< release a user (delete it only if there are no more active sessions

    bool setUserBlocked(const QString & aLogin, bool blockUser = true) const; //!< const cause it is the iUsers holds pointers so won't be changed

private:
    //! return user having more than aNbMaxOfNntpConByUser number of NntpConnection
    User *getUserHavingTooMuchNntpConnection_noLock(int aNbMaxOfNntpConByUser) const;

    inline void lockUser(User *aUser);  //!< Interface to lock user
    inline void unlockUser(User *aUser);//!< Interface to unlock user
    inline bool hasUserConnectionWithServer_noLock(User *aUser, ushort aServId); //!< interface to User non blocking funtion
    inline ushort getUserNumberOfNntpCons_noLock(User *aUser); //!< interface to User non blocking funtion
    inline vectNntpSrvOrderByCons getUserVectorOfNntpServerOrderedByNumberOfCons_noLock(User *aUser); //!< interface to User non blocking funtion
};

void UserManager::lockUser(User *aUser){aUser->lockNntpServList();}
void UserManager::unlockUser(User *aUser){aUser->unlockNntpServList();}
ushort UserManager::getUserNumberOfNntpCons_noLock(User *aUser){return aUser->getNumberOfNntpConnection_noLock();}
bool UserManager::hasUserConnectionWithServer_noLock(User *aUser, ushort aServId){
    return aUser->hasConnectionWithServer_noLock(aServId);
}
vectNntpSrvOrderByCons UserManager::getUserVectorOfNntpServerOrderedByNumberOfCons_noLock(User *aUser){
    return aUser->getVectorOfNntpServerOrderedByNumberOfConnections_noLock();
}

#endif // USERMANAGER_H
