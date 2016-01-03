#ifndef INPUTCONNECTION_H
#define INPUTCONNECTION_H

#include "connection.h"

/*!
 * \brief Server Side Nntp Connection (handle communication with the client that connects to the Proxy)
 */
class InputConnection : public Connection
{
    Q_OBJECT

public:
    explicit InputConnection(qintptr aSocketDescriptor); //!< Descriptor of the incoming socket
    InputConnection(const InputConnection &)              = delete;
    InputConnection(const InputConnection &&)             = delete;
    InputConnection & operator=(const InputConnection &)  = delete;
    InputConnection & operator=(const InputConnection &&) = delete;

    void closeConnection(); //!< How to close the connection

signals:
    void error(QString err); //!< signal errors (socket errors, authentication,...)
    void authenticated(std::string user, std::string pass); //!< Authentication steps done (but user not verified in DB)

public slots:
    void readyRead();        //!< Async Read, how to handle it
    void disconnected();     //!< What to do on socket disconnection
    bool doAuthentication(); //!< Launch the Authentication protocol (emit authenticated if success)
};

#endif // INPUTCONNECTION_H
