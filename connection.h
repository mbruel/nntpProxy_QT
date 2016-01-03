#ifndef CONNECTION_H
#define CONNECTION_H

#include "constants.h"
#include "nntpproxy.h" // inline log functions

#include <QObject>
#include <QTcpSocket>

QT_FORWARD_DECLARE_CLASS(QSslSocket)
QT_FORWARD_DECLARE_CLASS(QSslError)
QT_FORWARD_DECLARE_CLASS(QByteArray)


/*!
 * \brief represents one end of a TCP connection (either client or server side, SSL or not)
 * Abstract class. Children needs to implement the slots readyRead() and disconnected()
 * and the method closeConnection().
 *
 */
class Connection : public QObject
{
    Q_OBJECT

public:
    /*!
     * \brief Connection constructor
     * \param aSocketDescriptor : where to attach the socket in case of server connection
     * \param ssl               : should the connection be encrypted?
     * \param servSocket        : is it a client socket or a server one?
     * \param aClassName        : name of Child used for log purposes
     */
    explicit Connection(qintptr aSocketDescriptor, bool ssl = false,
                        bool servSocket = false, const char * aClassName = "Connection");
    Connection(const Connection &)              = delete;
    Connection(const Connection &&)             = delete;
    Connection & operator=(const Connection &)  = delete;
    Connection & operator=(const Connection &&) = delete;

    ~Connection();                        //!< destructor: delete the QTcpSocket

    virtual void   closeConnection() = 0; //!< how to close the connection
    inline qintptr getId() const;         //!< Connection id: iSocketDescriptor
    void           startAsyncRead();      //!< connect QTcpSocket::readyRead to local readyRead

    inline void    write(const QByteArray & aBuffer); //!< write on the socket
    inline void    setOutput(Connection *aOutputCon); //!< set iOutputCon
    inline QString getIpAddress() const;              //! return the Peer Ip Address


signals:
    void error(QTcpSocket::SocketError socketerror); //!< Socket Error
    void socketError(QString aError);                //!< Error during socket creation (ssl or not)

    void connected(); //!< TCP connection established (good ending of startTcpConnection)
    void closed();    //!< TCP socket is closed


public slots:
    virtual void readyRead()    = 0; //!< To be overridden in Child class
    virtual void disconnected() = 0; //!< To be overridden in Child class


    /*! \brief start the TCP connection.
     * If (aHost, aPort) are given, it's a client connection
     * Else it's a server connection that will be attached to the descriptor
     * Depending on isSsl, the connection will be encrypted
     *
     *  \return if the TCP connection is established
     */
    bool startTcpConnection(const char* aHost = NULL, ushort aPort = 0);

    void onEncryptedSocket();                         //!< SSL handshake done handler
    void onSslErrors(const QList<QSslError> &errors); //!< SSL errors handler
    void onErrors(QAbstractSocket::SocketError);      //!< Socket errors handler

protected:
    inline void _log(const QString &     aMessage) const; //!< log function for QString
    inline void _log(const char*         aMessage) const; //!< log function for char *
    inline void _log(const std::string & aMessage) const; //!< log function for std::string


private:
    bool createSslSocket(); //!< Create an SSL connection over the QTcpSocket

protected:
    qintptr     iSocketDescriptor; //!< socketDescriptor of input connection (used as connection id)
    bool        isSsl;             //!< is the connection encrypted?
    bool        isServerSocket;    //!< server socket or client socket?
    QTcpSocket *iSocket;           //!< Real TCP socket
    Connection *iOutputCon;        //!< Connection where what's read on the socket is forwarded
    QString     iLogPrefix;        //!< log prefix: Connection[<iSocketDescriptor>]
};


qintptr Connection::getId() const { return iSocketDescriptor;}

void Connection::setOutput(Connection *aOutputCon){iOutputCon = aOutputCon;}

void Connection::write(const QByteArray & aBuffer){iSocket->write(aBuffer);}

QString Connection::getIpAddress() const {return iSocket->peerAddress().toString();}

void Connection::_log(const char* aMessage) const {
     NntpProxy::log(iLogPrefix, aMessage);
}

void Connection::_log(const QString & aMessage) const {
     NntpProxy::log(iLogPrefix, aMessage);
}

void Connection::_log(const std::string & aMessage) const{
    NntpProxy::log(iLogPrefix, QString::fromStdString(aMessage));
}

#endif // CONNECTION_H
