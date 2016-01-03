#ifndef NNTPCONNECTION_H
#define NNTPCONNECTION_H

#include "connection.h"
#include "nntpserver.h"

/*!
 * \brief Nntp Client Connection (connect to a server with SSL or not)
 */
class NntpConnection  : public Connection
{
    Q_OBJECT

public:
    /*!
     * \brief only constructor authorized
     * \param aInputId: input socket id to link this connection to the input
     * \param aServer : server to connect to
     */
    explicit NntpConnection(qintptr aInputId,
                            const NntpServer & aServer);
    NntpConnection(const NntpConnection &)              = delete;
    NntpConnection(const NntpConnection &&)             = delete;
    NntpConnection & operator=(const NntpConnection &)  = delete;
    NntpConnection & operator=(const NntpConnection &&) = delete;

    inline ushort getServerId() const;            //!< return the server Id
    inline const QString & getServerHost() const; //!< return the server hostname
    inline ushort getServerPort() const;          //!< return the server port

    bool doAuthentication();              //!< do the Nntp Authentication steps

    inline ulong getDownloadSize() const; //!< return the downloaded size in Bytes (after authentication)
    inline uint getDownloadSizeMB() const;//!< return the downloaded size in MB (after authentication)

signals:
    void error(QString err); //!< signal errors (socket errors, authentication,...)
    void authenticated();    //!< Authentication succeed (server ready for commands)
    void serverRemoved();    //!< signal sent when the server is getting removed from the system

public slots:
    void readyRead();        //!< Async Read, how to handle it
    void disconnected();     //!< What to do on socket disconnection
    void closeConnection();  //!< How to close the connection

private:
    const NntpServer & iServer;        //!< handle to its server
    ulong              iDownloadSize;  //!< Bytes received (after authentication)

};

ushort NntpConnection::getServerId() const {return iServer.getId();}
const QString & NntpConnection::getServerHost() const{return iServer.getName();}
ushort NntpConnection::getServerPort() const{return iServer.getPort();}

ulong NntpConnection::getDownloadSize() const {return iDownloadSize;}
uint NntpConnection::getDownloadSizeMB() const {return iDownloadSize/1048576;}
#endif // NNTPCONNECTION_H
