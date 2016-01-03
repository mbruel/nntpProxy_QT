#ifndef NNTPSERVERMANAGER_H
#define NNTPSERVERMANAGER_H

#include "constants.h"
#include "mymanager.h"
#include "nntpserver.h"
#include "usermanager.h"

QT_FORWARD_DECLARE_CLASS(NntpConnection)
QT_FORWARD_DECLARE_CLASS(User)
QT_FORWARD_DECLARE_CLASS(QTcpSocket)
QT_FORWARD_DECLARE_CLASS(SessionManager)

/*!
 * \brief Manager that OWNS all the actives NntpServers
 * - owns the NntpServers (delete them on removal)
 */
class NntpServerManager : public MyManager<NntpServer>
{
public:
    friend class SessionManager; //!< SessionManager::tryToGetNntpConnectionFromOtherUser is able to lock/unlock

    //!< Constructor with a list of NntpServerParameters (from parsing config file)
    explicit NntpServerManager(const QVector<NntpServerParameters *> &aServParams, UserManager & aUserMgr);
    NntpServerManager(const NntpServerManager &)              = delete;
    NntpServerManager(const NntpServerManager &&)             = delete;
    NntpServerManager & operator=(const NntpServerManager &)  = delete;
    NntpServerManager & operator=(const NntpServerManager &&) = delete;

    short addNntpServer(const NntpServerParameters &aParam); //!< return -1 on error or the id of the created server (Thread_Safe)
    bool removeNntpServer(ushort aServerId);                 //!< remove a NntpServer and delete it (Thread_Safe)

    inline ushort getMaxNumberOfConnections() const;       //!< return max number of connections (Thread_Safe)
    inline ushort getNumberOfConnectionsAvailable() const; //!< return number of connections available (Thread_Safe)
    inline ushort getNumberOfConnectionsInUse() const;     //!< return max number of connections in use (Thread_Safe)

    // -1 if server aServerId is not found
    inline int getMaxNumberOfConnections(ushort aServerId) const;       //!< return max num of cons of Server (Thread_Safe)
    inline int getNumberOfConnectionsAvailable(ushort aServerId) const; //!< return num of cons available pf Server(Thread_Safe)
    inline int getNumberOfConnectionsInUse(ushort aServerId) const;     //!< return max num of cons in use of Server(Thread_Safe)


    /*!
     * \brief provide a NntpConnection for a given User (if some available, from Server where the user has the less connections)
     * \param aInputConId : SessionHandler Id (used for log purposes)
     * \param aUser : User that ask for a connection
     * \return
     */
    NntpConnection *getNntpConnection(qintptr aInputConId, User *aUser);
    NntpConnection *getMonitoringNntpConnection(); //!< TODO: for monitor server (no user needed)

    /*!
     * \brief release a NntpConnection via its server (Thread_Safe by default)
     * \param aCon     : connection to release
     * \param useMutex : Should it be blocking?
     * \return
     */
    bool releaseNntpConnection(NntpConnection *aCon, bool useMutex = true);

    //!< Check if all the servers can use all their connections at the same time
    bool canConnectToNntpServers();


private:
    //! Factoring function to get the number of connection depending on the type
    ushort getNumberOfConnections(NntpServer::TypeOfConnectionNumber aTypeOfConnection) const;

    //! Factoring code to get the number of connection of a specific server depending on the type
    int    getNumberOfConnections(ushort aServerId, NntpServer::TypeOfConnectionNumber aTypeOfConnection) const;

    void lockAllServers();   //!< Lock all servers (block their list of connections)
    void unlockAllServers(); //!< Unlock all servers (block their list of connections)

    /*!
     * \brief Used by SessionManager::tryToGetNntpConnectionFromOtherUser to get a NntpConnection from a Server
     * \param aInputConId : InputId for the created NntpConnection
     * \param aServId     : Server from which we get the NntpConnection (one just got released)
     * \return
     */
    NntpConnection *getOfferedNntpConnectionFromServer_noLock(qintptr aInputConId, ushort aServId);

private:
    UserManager & iUserMgr;     //!< UserManager handle to be able to lock users
    ushort iNumberNntpConMax;   //!< Number max of connections
    ushort iNumberNntpConInUse; //!< Global number of used connections (to avoid to go through the list of servers)

};



ushort NntpServerManager::getMaxNumberOfConnections() const{
    return getNumberOfConnections(NntpServer::TypeOfConnectionNumber::MaxNuber);
}

ushort NntpServerManager::getNumberOfConnectionsAvailable() const{
    return getNumberOfConnections(NntpServer::TypeOfConnectionNumber::Available);
}

ushort NntpServerManager::getNumberOfConnectionsInUse() const{
    return getNumberOfConnections(NntpServer::TypeOfConnectionNumber::InUse);
}


int NntpServerManager::getMaxNumberOfConnections(ushort aServerId) const{
    return getNumberOfConnections(aServerId, NntpServer::TypeOfConnectionNumber::MaxNuber);
}

int NntpServerManager::getNumberOfConnectionsAvailable(ushort aServerId) const{
    return getNumberOfConnections(aServerId, NntpServer::TypeOfConnectionNumber::Available);
}

int NntpServerManager::getNumberOfConnectionsInUse(ushort aServerId) const{
    return getNumberOfConnections(aServerId, NntpServer::TypeOfConnectionNumber::InUse);
}

#endif // NNTPSERVERMANAGER_H
