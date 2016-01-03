#include "nntpserver.h"
#include "nntpconnection.h"
#include "nntpproxy.h"

#include <QTcpSocket>
#include <QList>

unsigned short NntpServer::sNextId = 0;

NntpServer::NntpServer(const NntpServerParameters & aParams):
    iParams(aParams), iId(sNextId++), iNntpCons(), mMutex(),
    iLogPrefix(QString("NntpServer").append("[").append(QString::number(iId)).append("] "))
{
#ifdef LOG_CONSTRUCTORS
    _log("Constructor");
#endif
    iNntpCons.reserve(aParams.maxConnections);
}

NntpServer::~NntpServer(){
    mMutex.lock();
#ifdef LOG_CONSTRUCTORS
    QString str("Destructor, ");
    str += getSizeStr_noLock();
    _log(str);
#endif
    for (int i=0; i<iNntpCons.size(); ++i){
        // we don't own the NntpConnection, SessionHandler does
        // we send a signal so the whole session will be closed properly
        emit iNntpCons[i]->serverRemoved();
    }
    iNntpCons.clear();
    mMutex.unlock();
}

QTextStream &  operator<<(QTextStream & stream, const NntpServer &aServer){
    stream << aServer.iLogPrefix;
    return stream;
}

NntpConnection* NntpServer::getNntpConnection(qintptr aInputId){
    QMutexLocker lock(&mMutex);
    if (iNntpCons.size() >= iParams.maxConnections){
        _log("Error getNntpConnection: can't provide a connection as they're all used already");
        return Q_NULLPTR;
    }

    NntpConnection *con = new NntpConnection(aInputId, *this);
    iNntpCons.append(con);

    QTextStream &is = NntpProxy::acquireLog(iLogPrefix);
    is << "Adding new Nntp Connection with id: " << aInputId
       << ", " << getSizeStr_noLock();
    NntpProxy::releaseLog();

    return con;
}

NntpConnection* NntpServer::getNntpConnection_noLock(qintptr aInputId){
    if (iNntpCons.size() >= iParams.maxConnections){
        _log("Error getNntpConnection: can't provide a connection as they're all used already");
        return Q_NULLPTR;
    }

    NntpConnection *con = new NntpConnection(aInputId, *this);
    iNntpCons.append(con);

    QTextStream &is = NntpProxy::acquireLog(iLogPrefix);
    is << "Adding new Nntp Connection with id: " << aInputId
       << ", " << getSizeStr_noLock();
    NntpProxy::releaseLog();

    return con;
}

bool NntpServer::releaseNntpConnection(NntpConnection *aNntpCon){
    if (aNntpCon->getServerId() != iId){
        _log("Error releaseNntpConnection: trying to release a connection to the wrong server...");
        return false;
    }

    QMutexLocker lock(&mMutex);
    bool out = iNntpCons.removeOne(aNntpCon);

    QTextStream &is = NntpProxy::acquireLog(iLogPrefix);
    is << "Release Nntp Connection with id: " << aNntpCon->getId()
       << ", result: " << out
       << ", " << getSizeStr_noLock();
    NntpProxy::releaseLog();

    return out;
}



bool NntpServer::canUseAllConnections() {
    bool canUseAllConnections = true;

    ushort maxCon = getMaxNumberOfConnections();
    QString str("canUseAllConnection, try to open the max number of allowed connection: ");
    str += QString::number(maxCon);
    _log(str);

    QList<NntpConnection *> cons;
    for (int i=0; i<maxCon; ++i){
        NntpConnection * con = getNntpConnection(i);
        cons.append(con);

        if (!con->startTcpConnection(iParams.name.toLatin1().constData(), iParams.port)){
            QString err("Error canUseAllConnections, the connection #");
            err += QString::number(i);
            err += " failed to connect...";
            _log(err);
            canUseAllConnections = false;
            break;
        }
        if (iParams.auth && !con->doAuthentication()){
            QString err("Error canUseAllConnections, the connection #");
            err += QString::number(i);
            err += " failed to authenticate...";
            _log(err);
            canUseAllConnections = false;
            break;
        }
    }

    for (int i=0; i < cons.size(); ++i){
        releaseNntpConnection(cons[i]);
        delete cons[i];
    }

    cons.clear();

    return canUseAllConnections;
}
