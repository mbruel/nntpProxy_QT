#ifndef USER_H
#define USER_H

#include "constants.h"

#include <map>
#include <vector>

#include <QMutex>
#include <QMutexLocker>

QT_FORWARD_DECLARE_CLASS(QTextStream)


//! Function to order a pair<ushort, ushort> by values
bool compNntpServConsByValue(const std::pair<ushort,ushort>& lhs, const std::pair<ushort,ushort>& rhs);


//! vector of pair<key,val> of map iNntpServCons ordered by value (number of connections)
typedef std::vector<std::pair<ushort,ushort>> vectNntpSrvOrderByCons;


/*!
 * \brief a User is defined by the pair (IP, login)
 * A user can have multiple InputConnections that will have a corresponding NntpConnection
 * it holds a map iNntpServCons to know the distribution of the NntpConnection among the availabe NntpServers
 */
class User
{
public:

    friend class UserManager; //!< Manager is able to lock/unlock the User

#ifdef TESTUSER_H
    friend class TestUser;
#endif
    /*!
     * \brief User explicit Constructor
     * \param aIpAddress: user IP
     * \param aLogin:     user login
     */
    explicit User(const QString & aIpAddress, const QString & aLogin);
    User(const User &)              = delete;
    User(const User &&)             = delete;
    User & operator=(const User &)  = delete;
    User & operator=(const User &&) = delete;

    ~User(); //!< Destructor

    /*!
     * \brief operator == using pair (iLogin, iIpAddress)
     * \param aUser
     * \return
     */
    bool operator== (const User & aUser) const;

    inline const QString & getLogin() const; //!< return user login
    inline const QString & getId() const;    //!< return user login (needed by MyManager template)

    inline const QString & getIp() const;    //!< return user IP
    inline ushort          getDbId() const;  //!< return user id stored in the Database

    //!< To be able to print a User
    friend QTextStream &  operator<<(QTextStream & stream, const User &aUser);

    QString str() const; //!< User info as a string (mainly used for unit testing TestUser)

    QString downSize() const; //!< return human readable download size

    inline void setBlocked(bool block); //!< Edit iIsBlocked
    inline void setDbId(ushort aId);    //!< Database will set the db id

    inline void  addDownloadSize(ulong aDownloadSize); //!< Add download size (in Bytes) Thread_Safe
    inline ulong getDownloadedSize() const;            //!< return the downloaded size   Thread_Safe

    inline void   newInputConnection(); //!< Add a new input connection Thread_Safe
    inline void   delInputConnection(); //!< Remove an input connection Thread_Safe
    inline ushort getNumberOfInputConnection() const; //!< Get the number of Input Connections Thread_Safe

    void newNntpConnection(ushort aServerId); //!< Add a new Nntp connection Thread_Safe
    bool delNntpConnection(ushort aServerId); //!< Remove an Nntp connection Thread_Safe

private:
    inline ushort getNumberOfNntpConnection_noLock() const; //!< Get the number of Nntp Connections
    inline bool hasConnectionWithServer_noLock(ushort aServerId) const; //!< Return if the user has any Nntp Connections with the serverId

    //! return a vector of pair<key.val> of the map iNntpServCons ordered by values (Not Thread_Safe, Used by Manager)
    vectNntpSrvOrderByCons getVectorOfNntpServerOrderedByNumberOfConnections_noLock() const;

    inline void lockNntpServList();  //!< Used by Manager to lock iNntpServCons
    inline void unlockNntpServList();//!< Used by Manager to unlock iNntpServCons

private:
    const QString            iIpAddress;    //!< IP address
    const qint64             iStartTimeMs;  //!< timestamp of first connection

    ushort                   iId;           //!< will get it from DB (so not const)
    const QString            iLogin;        //!< User login
    bool                     isBlocked;     //!< From DB (no payment) but changeable live

    ushort                   iNumInputCons; //!< number of input connections (or threads)
    ushort                   iNumNntpCons;  //!< number of nntp connections
    mutable QMutex           mNumInput;     //!< Thread safety for iNumInputCons
    mutable QMutex           mNumNntp;      //!< Thread safety for iNumNntpCons

    ulong                    iDownloadSize; //!< total download size (via all its threads)
    mutable QMutex           mDownSize;     //!< Thread safety iDownloadSize

    std::map<ushort, ushort> iNntpServCons; //!< (server_id, number of connections)
};


const QString & User::getLogin() const {return iLogin;}
const QString & User::getId() const {return iLogin;}

const QString & User::getIp() const {return iIpAddress;}
ushort          User::getDbId() const {return iId;}

void User::setBlocked(bool block){isBlocked = block;}
void User::setDbId(ushort aId){iId = aId;}

void   User::addDownloadSize(ulong aDownloadSize){QMutexLocker lock(&mDownSize); iDownloadSize += aDownloadSize;}
ulong  User::getDownloadedSize() const {QMutexLocker lock(&mDownSize); return iDownloadSize;}

void   User::newInputConnection(){QMutexLocker lock(&mNumInput); ++iNumInputCons;}
void   User::delInputConnection(){QMutexLocker lock(&mNumInput); --iNumInputCons;}
ushort User::getNumberOfInputConnection() const {QMutexLocker lock(&mNumInput); return iNumInputCons;}

ushort User::getNumberOfNntpConnection_noLock() const {return iNumNntpCons;}


bool User::hasConnectionWithServer_noLock(ushort aServerId) const {
    return iNntpServCons.find(aServerId) != iNntpServCons.end();
}

void User::lockNntpServList(){mNumNntp.lock();}
void User::unlockNntpServList(){mNumNntp.unlock();}

#endif // USER_H
