#include "usermanager.h"
#include "user.h"
#include "nntpproxy.h"
#include "database.h"

#include <QTcpSocket>

UserManager::UserManager(): MyManager<User>("User"){}

UserManager::~UserManager(){}


User * UserManager::addUser(const QString & aIpAddress, const QString & aLogin){
    User * tmpUser = new User(aIpAddress, aLogin);

    // Mutex for both find and append
    QMutexLocker lock(mMutex);

    User * user = find(tmpUser, false);
    if (user != Q_NULLPTR)
        delete tmpUser;
    else {
        user = tmpUser;
        iList.append(user);
    }
    user->newInputConnection();
    return user;
}


bool UserManager::releaseUser(User *aUser, Database &aDb){
    QTextStream &ostream = NntpProxy::acquireLog(iLogPrefix);
    ostream << "releasing user: " << *aUser;
    NntpProxy::releaseLog();

    // User has not been inserted via addUser
    if (aUser->getNumberOfInputConnection() == 0) {
        _log("Error: user didn't have any input connection");
        return false;
    }

    aUser->delInputConnection();

    if (aUser->getNumberOfInputConnection() == 0){

        bool err = iList.removeOne(aUser);
        if (!err)
            _log(QString("Error removing user: ").append(aUser->getLogin()));

        aDb.addUserSize(aUser);
        delete aUser;
        _log("> user deleted...");
        return err;
    } else {
        // User has some connections left
        _log("> user has still active thread...");
        return true;
    }
}



bool UserManager::setUserBlocked(const QString & aLogin, bool blockUser) const{
    bool userFound = false;
    for (QList<User *>::const_iterator it = iList.cbegin(); it != iList.cend(); ++it){
        if ((*it)->getLogin() == aLogin){
            userFound = true;
            (*it)->setBlocked(blockUser);
            // We don't break as we want to block the user for all Ips
        }
    }
    return userFound;
}


User *UserManager::getUserHavingTooMuchNntpConnection_noLock(int aNbMaxOfNntpConByUser) const{
    User * user = Q_NULLPTR;
    ushort maxNbNntpCon = 0;
    for (int i=0; i<iList.size(); ++i){        
        ushort nbCon = iList[i]->getNumberOfNntpConnection_noLock();
        if ( (nbCon > aNbMaxOfNntpConByUser) && (nbCon > maxNbNntpCon) ){
            maxNbNntpCon = nbCon;
            user = iList[i];
        }
    }
    return user;
}
