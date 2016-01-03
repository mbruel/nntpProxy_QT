#ifndef NNTPSERVER_H
#define NNTPSERVER_H

#include "constants.h"
#include "nntpproxy.h"

#include <QMutex>
#include <QMutexLocker>
#include <QList>

QT_FORWARD_DECLARE_CLASS(NntpConnection)
QT_FORWARD_DECLARE_CLASS(QTcpSocket)
QT_FORWARD_DECLARE_CLASS(QTextStream)


/*!
 * \brief NntpServer represents an NntpServer we can connect to
 * - initialised via an NntpServerParameters created by parsing the config file.
 * - holds but NOT owns a list of handles of all the NntpConnection currently in use by the users.
 * - creates the NntpConnections to offer them to users (if some still available).
 * - is NOT responsible for the destruction of the NntpConnection (SessionHandler is)
 */
class NntpServer
{
public:
    friend class NntpServerManager; //!< Manager is able to lock/unlock the NntpServer

#ifdef TESTNNTPSERVER_H
    friend class TestNntpServer; //!< simulate NntpServerManager
#endif

    enum TypeOfConnectionNumber{ //!< Type of number of connections that can be requested
        MaxNuber = 0,
        InUse = 1,
        Available = 2
    };

    explicit NntpServer(const NntpServerParameters & aParams); //!< Constructor via a NntpServerParameters
    NntpServer(const NntpServer &)              = delete;
    NntpServer(const NntpServer &&)             = delete;
    NntpServer & operator=(const NntpServer &)  = delete;
    NntpServer & operator=(const NntpServer &&) = delete;

    ~NntpServer(); //!< send signals to all its NntpConnection to get them closed (other Threads)

    inline ushort getId() const;                //!< return the server id (needed by MyManager template)
    inline const QString& getName() const;      //!< return the server name
    inline ushort getPort() const;              //!< return the server port
    inline const QString & getAuthUser() const; //!< return server user
    inline const QString & getAuthPass() const; //!< return server pass
    inline bool isSsl() const;                  //!< return if the server connection should be encrypted

    //!< To be able to print a NntpServer
    friend QTextStream &  operator<<(QTextStream & stream, const NntpServer &aServer);

    inline ushort getMaxNumberOfConnections() const;       //!< Maximum number of connections (from param in construction)
    inline ushort getNumberOfConnectionsAvailable() const; //!< number of connections currently available
    inline ushort getNumberOfConnectionsInUse() const;     //!< number of connections currently in use
    inline bool   hasConnectionAvailable() const;          //!< is there any connections currently available


    NntpConnection* getNntpConnection(qintptr aInputId);  //!< provides an NntpConnection if there are still some available
    bool releaseNntpConnection(NntpConnection *aNntpCon); //!< release an NntpConnection but doesn't delete it

    bool canUseAllConnections(); //!< Check if we can use all the NntpConnections at the same time

private:
    inline void _log(const QString &     aMessage) const; //!< Add a log line
    inline void _log(const char*         aMessage) const; //!< Add a log line
    inline QString  getSizeStr_noLock() const;            //!< get String of the list size

    ///////////////////////
    /// Funtions to be used only by friend NntpServerManger
    ///
    inline ushort   getNumberOfConnectionsAvailable_noLock() const; //!< number of connections currently available
    inline ushort   getNumberOfConnectionsInUse_noLock() const;     //!< number of connections currently in use
    inline bool     hasConnectionAvailable_noLock() const;          //!< is there any connections currently available
    NntpConnection* getNntpConnection_noLock(qintptr aInputId);     //!< give an NntpConnection to be used


private:
    static ushort              sNextId;    //!< Next id for a new server (auto-increment)

    const NntpServerParameters iParams;    //!< server parameters (from config.xml)
    const ushort               iId;        //!< Server id

    QList<NntpConnection *>    iNntpCons;  //!< List of all the connections currently in use
    mutable QMutex             mMutex;     //!< thread safe iNntpCons

    const QString              iLogPrefix; //!< log prefix
};


ushort NntpServer::getId() const {return iId;}
const QString & NntpServer::getName() const {return iParams.name;}
ushort NntpServer::getPort() const {return iParams.port;}

const QString & NntpServer::getAuthUser() const{return iParams.login;}
const QString & NntpServer::getAuthPass() const{return iParams.pass;}
bool NntpServer::isSsl() const {return iParams.ssl;}

QString NntpServer::getSizeStr_noLock() const{
    QString str("Available connection: ");
    str += QString::number(getNumberOfConnectionsAvailable_noLock());
    str += " / ";
    str += QString::number(iParams.maxConnections);
    return str;
}

ushort NntpServer::getMaxNumberOfConnections() const { return iParams.maxConnections;}
ushort NntpServer::getNumberOfConnectionsAvailable() const {
    QMutexLocker lock(&mMutex);
    return iParams.maxConnections - iNntpCons.size();
}
ushort NntpServer::getNumberOfConnectionsInUse() const {
    QMutexLocker lock(&mMutex);
    return iNntpCons.size();
}
bool NntpServer::hasConnectionAvailable() const {
    QMutexLocker lock(&mMutex);
    return (iNntpCons.size() < iParams.maxConnections);
}

ushort NntpServer::getNumberOfConnectionsAvailable_noLock() const {
    return iParams.maxConnections - iNntpCons.size();
}
ushort NntpServer::getNumberOfConnectionsInUse_noLock() const {
    return iNntpCons.size();
}
bool NntpServer::hasConnectionAvailable_noLock() const {
    return (iNntpCons.size() < iParams.maxConnections);
}

void NntpServer::_log(const char* aMessage) const {
     NntpProxy::log(iLogPrefix, aMessage);
}

void NntpServer::_log(const QString & aMessage) const {
     NntpProxy::log(iLogPrefix, aMessage);
}

#endif // NNTPSERVER_H
