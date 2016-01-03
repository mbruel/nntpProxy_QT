#ifndef SessionHandler_H
#define SessionHandler_H

#include "constants.h"
#include "nntpproxy.h"

#include <QObject>


QT_FORWARD_DECLARE_CLASS(InputConnection)
QT_FORWARD_DECLARE_CLASS(SessionManager)
QT_FORWARD_DECLARE_CLASS(NntpProxy)
QT_FORWARD_DECLARE_CLASS(MyThread)
QT_FORWARD_DECLARE_CLASS(NntpConnection)
QT_FORWARD_DECLARE_CLASS(User)
QT_FORWARD_DECLARE_CLASS(QTextStream)

#include <QWaitCondition>
#include <QMutex>


/*!
 * \brief SessionHandler runs in a Thread and take care of the whole user session
 * - owns the InputConnection and the output NntpConnection
 * - DOESN't own the User (as it is shared between multiple sessions)
 */
class SessionHandler : public QObject
{
    Q_OBJECT

public:
    friend class SessionManager; //!< Manager is able to lock/unlock the Session

    /*!
     * \brief SessionHandler constructor
     * \param aSocketDescriptor: where to attach the socket
     * \param aInputMgr : handle on session manager to getUser, getNntpConnection...
     * \param aThread : handle on running thread (Not own it but could wait for it on main thread)
     */
    explicit SessionHandler(qintptr aSocketDescriptor, SessionManager & aInputMgr,
                            MyThread *aThread);
    SessionHandler(const SessionHandler &)              = delete;
    SessionHandler(const SessionHandler &&)             = delete;
    SessionHandler & operator=(const SessionHandler &)  = delete;
    SessionHandler & operator=(const SessionHandler &&) = delete;

    ~SessionHandler();            //!< Release all the allocated resources, Connetions, User...
    inline qintptr getId() const; //!< return iSocketDescriptor (needed by MyManager template)

    //!< To be able to print a NntpServer
    friend QTextStream &  operator<<(QTextStream & stream, const SessionHandler &aSession);

public slots:
    void closeSession(); //!< connects to &SessionHandler::stopSession (to shutdown session from another Thread)

    void handleSocketError(QString aError);    //!< connects to &InputConnection::socketError
    void handleNntpSocketError(QString aError);//!< connects to &NntpConnection::socketError

    void inputAuthenticated(std::string aLogin, std::string aPass); //!< connects to &InputConnection::authenticated

    void closeNntpConnection(); //!< connects to &NntpConnection::closed
    void nntpServerRemoved();   //!< connects to &NntpConnection::serverRemoved

signals:
    void startConnection(const char* aHost=NULL, ushort aPort=0); //!< trigger &Connection::startTcpConnection
    void stopSession();   //!< trigger &SessionHandler::closeSession
    void deleteSession(); //!< trigger &QObject::deleteLater to suicide
    void destroyed();     //!< &QThread::quit to delete thread from main Thread


private:
    inline void _log(const QString & aMessage) const; //!< Add a log line
    inline void _log(const char*     aMessage) const; //!< Add a log line

    void startForwarding(); //!< Start the proxy job (forwarding commands/responses from iInputCon to iNntpCon)

    NntpConnection * offerNntpConnection(); //!< Used by friend and owner SessionManager

    void waitNntpSessionClosed(); //!< From other thread, wait for the NntpSession to be closed so we can offer it to another user
    void waitForDeletion();       //!< From main thread, on Proxy shutdown, we wait for the session to finish properly

private:
    qintptr           iSocketDescriptor; //!< Input Socket descriptor used as Session id
    InputConnection  *iInputCon;         //!< input connection (owns it)
    SessionManager  & iSessionMgr;       //!< Handle on manager
    MyThread         *iThread;           //!< Handle on Thread it is running in
    NntpConnection   *iNntpCon;          //!< nntp connnection (owns it)
    User             *iUser;             //!< handle on user (DOES NOT own it, UserManager does)
    bool             isActive;           //!< in order to close the session only once (if we get several socket errors...)
    const QString    iLogPrefix;         //!< log prefix

    bool isForwarding;                   //!< Do we have an NntpConnection?
    bool isNntpServerActive;             //!< is the NntpServer still active?

    // To handle properly closing from other thread when the Nntp connection is offered
    QMutex         *mNntpConOffered; //!< Mutex to close Session from another thread when the NntpCon is offered
    QWaitCondition *wNntpConOffered; //!< WaitCond to close Session from another thread when the NntpCon is offered
    bool            isNntpConOffered;//!< bool to know if the NntpCon is beeing offered

    // To handle shutdown properly
    QMutex         *mShutdownManager; //!< Mutex to close Session properly from Main Thread on Shutdown
    QWaitCondition *wShutdownManager; //!< WaitCond to close Session properly from Main Thread on Shutdown
    bool            isShutdownManager;//!< bool to know if the session manager is shutting down

};

qintptr SessionHandler::getId() const {return iSocketDescriptor;}

void SessionHandler::_log(const char* aMessage) const {
     NntpProxy::log(iLogPrefix, aMessage);
}

void SessionHandler::_log(const QString & aMessage) const {
     NntpProxy::log(iLogPrefix, aMessage);
}

#endif // SessionHandler_H
