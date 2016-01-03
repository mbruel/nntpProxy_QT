#include "nntpservermanager.h"
#include "nntpserver.h"
#include "nntpconnection.h"
#include "user.h"
#include "usermanager.h"

NntpServerManager::NntpServerManager(const QVector<NntpServerParameters *> &aServParams, UserManager & aUserMgr):
    MyManager<NntpServer>("NntpServer"), iUserMgr(aUserMgr),
    iNumberNntpConMax(0), iNumberNntpConInUse(0)
{
    for (int i=0; i<aServParams.size(); ++i){
        iList.append(new NntpServer(*(aServParams[i])));
        iNumberNntpConMax += aServParams[i]->maxConnections;
    }
}




bool NntpServerManager::canConnectToNntpServers(){
    QMutexLocker lock(mMutex);

    bool canConnectToAllServers = true;
    for (int i=0; i<iList.size(); ++i){
        if (!iList[i]->canUseAllConnections()){
            QString err("Error, can't use all connection of server: ");
            err += QString::number(iList[i]->getId());
            _log(err);
            canConnectToAllServers =false;
            break;
        }
    }
    return canConnectToAllServers;
}


short NntpServerManager::addNntpServer(const NntpServerParameters &aParam){
    NntpServer *serv = new NntpServer(aParam);
    if (!serv){
        _log("Error allocating new Server...");
        return -1;
    }

    if (!serv->canUseAllConnections()){
        _log("Error trying to use all the server connection...");
        return -1;
    }

    QMutexLocker lock(mMutex);
    iList.append(serv);
    iNumberNntpConMax += aParam.maxConnections;
    return serv->iId;
}



bool NntpServerManager::removeNntpServer(ushort aServerId){
    // Mutex for both find and erase
    QMutexLocker lock(mMutex);

    NntpServer *serv = find(aServerId, false);
    if (serv == Q_NULLPTR){
        QString str("Error removing Nntp Server (not found) with id=");
        str += QString::number(aServerId);
        _log(str);
        return false;
    }

    erase(serv, false);

    iNumberNntpConMax   -= serv->getMaxNumberOfConnections();
    iNumberNntpConInUse -= serv->getNumberOfConnectionsInUse();
    delete serv;
    return true;
}


ushort NntpServerManager::getNumberOfConnections(NntpServer::TypeOfConnectionNumber aTypeOfConnection) const {
    QMutexLocker lock(mMutex);

    switch (aTypeOfConnection) {
    case NntpServer::TypeOfConnectionNumber::MaxNuber :
        return iNumberNntpConMax;
    case NntpServer::TypeOfConnectionNumber::InUse :
        return iNumberNntpConInUse;
    case NntpServer::TypeOfConnectionNumber::Available :
        return iNumberNntpConMax - iNumberNntpConInUse;
    }

    return 0;
}



int NntpServerManager::getNumberOfConnections(
        ushort aServerId, NntpServer::TypeOfConnectionNumber aTypeOfConnection) const{

    QMutexLocker lock(mMutex);

    NntpServer * serv = find(aServerId, false);
    if (serv == Q_NULLPTR){
        QString err("Error: There is no server with id: ");
        err += QString::number(aServerId);
        _log(err);
        return -1;
    }

    switch (aTypeOfConnection) {
    case NntpServer::TypeOfConnectionNumber::MaxNuber :
        return serv->getMaxNumberOfConnections();
    case NntpServer::TypeOfConnectionNumber::InUse :
        return serv->getNumberOfConnectionsInUse();
    case NntpServer::TypeOfConnectionNumber::Available :
        return serv->getNumberOfConnectionsAvailable();
    }

    return -1;
}








bool NntpServerManager::releaseNntpConnection(NntpConnection *aCon, bool useMutex){
    ushort servId = aCon->getServerId();
    bool conReleased = false;

    // Mutex for both find and erase
    if (useMutex)
        mMutex->lock();

    NntpServer *serv = find(servId, false);
    if (serv == Q_NULLPTR){
        QString err("Error can't find the connection server with id: ");
        err += QString::number(servId);
        _log(err);
    } else {
        if (serv->releaseNntpConnection(aCon) ){
            --iNumberNntpConInUse;
            conReleased = true;
        }
    }

    // Mutex for both find and erase
    if (useMutex)
        mMutex->unlock();

    return conReleased;

}

void NntpServerManager::lockAllServers(){
    for (int i=0; i< iList.size(); ++i)
        iList[i]->mMutex.lock();
}

void NntpServerManager::unlockAllServers(){
    for (int i=0; i< iList.size(); ++i)
        iList[i]->mMutex.unlock();
}



NntpConnection *NntpServerManager::getNntpConnection(qintptr aInputConId, User *aUser){

    QMutexLocker lock(mMutex);
    if (iNumberNntpConInUse == iNumberNntpConMax){
        _log("All the connections are already in use...");
        return Q_NULLPTR;
    }

    lockAllServers();
    iUserMgr.lockUser(aUser);

    ushort nbServers    = iList.size();
    NntpConnection *con = Q_NULLPTR;

    // 1.:
    // find all the Nntp Servers with connections available
    // that the user doesn't use yet
    QList<NntpServer *> unusedServers;
    unusedServers.reserve(nbServers);
    for (int i=0; i< nbServers;  ++i){
        NntpServer *srv = iList[i];
        if (srv->hasConnectionAvailable_noLock()
                && !iUserMgr.hasUserConnectionWithServer_noLock(aUser, srv->getId())){
            unusedServers.append(srv);
        }
    }

    // 2:
    // if there are servers that the user doesn't use yet,
    // we give the one with the most available connections
    if (unusedServers.size() > 0) {
        ushort maxConAvailable = 0;
        NntpServer *servMaxConAv = Q_NULLPTR;
        for (int i=0; i < unusedServers.size(); ++i){
            if (unusedServers[i]->getNumberOfConnectionsAvailable_noLock() > maxConAvailable){
                servMaxConAv    = unusedServers[i];
                maxConAvailable = servMaxConAv->getNumberOfConnectionsAvailable_noLock();
            }
        }
        _log("we found a NEW connection for the user");
        con = servMaxConAv->getNntpConnection_noLock(aInputConId);
    }
    // 3.: Otherwise if no unused server
    // we take a connection from the server that the user has less
    // (We get the list of serverId ordered by number min of connection
    // and we take a connection from the first server that has some available)
    else {
        vectNntpSrvOrderByCons userServCon = iUserMgr.getUserVectorOfNntpServerOrderedByNumberOfCons_noLock(aUser);
        for (auto it = userServCon.cbegin(); it!= userServCon.cend(); ++it){
            ushort servId = it->first;
            NntpServer * serv = find(servId, false); // non blocking
            if (serv == Q_NULLPTR){
                QTextStream &ostream = NntpProxy::acquireLog(iLogPrefix);
                ostream << "Error getting new Nntp Connection for user: " << aUser
                        << ", it is still using a server that is not anymore active..."
                        << " (serverId: " << servId;
                NntpProxy::releaseLog();
                continue;
            }
            if (serv->hasConnectionAvailable_noLock()){
                _log("we found a connection for the user");
                con = serv->getNntpConnection_noLock(aInputConId);
                break;
            }
        }
    }

    if (con!=Q_NULLPTR)
        ++iNumberNntpConInUse;

    iUserMgr.unlockUser(aUser);
    unlockAllServers();

    return con;
}

NntpConnection *NntpServerManager::getOfferedNntpConnectionFromServer_noLock(qintptr aInputConId, ushort aServId){
    _log("getOfferedNntpConnectionFromServer_noLock");
    NntpServer *serv = find(aServId, false);
    if (serv == Q_NULLPTR){
        QString err("Error can't find the server with id: ");
        err += QString::number(aServId);
        _log(err);
        return Q_NULLPTR;
    }

    return serv->getNntpConnection_noLock(aInputConId);;
}


NntpConnection *NntpServerManager::getMonitoringNntpConnection(){
    NntpConnection *con = Q_NULLPTR;
// TODO
    return con;
}
