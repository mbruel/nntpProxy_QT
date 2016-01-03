#ifndef NNTPPROXY_H
#define NNTPPROXY_H

#include "constants.h"
#include "log.h"
#include "nntp.h"
#include "mycrypt.h"

#include <QtNetwork/QTcpServer>

QT_FORWARD_DECLARE_CLASS(SessionManager)
QT_FORWARD_DECLARE_CLASS(SessionHandler)
QT_FORWARD_DECLARE_CLASS(Log)
QT_FORWARD_DECLARE_CLASS(UserManager)
QT_FORWARD_DECLARE_CLASS(Database)
QT_FORWARD_DECLARE_CLASS(NntpServerManager);



/*!
 * \brief Multi-Threaded multi output Nntp Proxy that spread equaly the fixed output connection between users.
 * - Follows singleton pattern
 * - Multi Users (DB authentication)
 * - Max number of connections per User
 * - Can use several outputs (NntpServers) that have a fixed number of connections
 * - Distribute the outputs of users using all differents outputs available (balancing)
 * - When all the outputs are in use, balance them between the different users connected
 * (Users can loose connections if new users connects so they all get the average available)
 */
class NntpProxy : public QTcpServer
{
    Q_OBJECT

public:
    bool startProxy(char * aConfigFile = NULL); //!< start the proxy with the xml config file (default one in constants.h)
    static void shutDown(); //!< Stop the proxy, delete the instance and all allocated resources

    static bool initStatics(char * aConfigFile = NULL); //!< Initialise the statics (parse config file, open Log file...)
    bool init(char * aConfigFile = NULL); //!< Initialise the statics and the proxy instance without launching the server

    static NntpProxy *getInstance(QObject *parent = 0); //!< get the Proxy instance


    inline static bool isClientSSL(); //!< Are the clients using SSL connection (from congig file)
    inline static LOG_LEVEL logLevel(); //!< return the log level
    inline static ushort getMaxConnectionsPerUser(); //!< return the maximum number of connection per user (from config file)

    //! Acquire the Log file (locking it) and writing a new line with a prefix
    inline static QTextStream& acquireLog(const char *    aAcquirerName);
    //! Acquire the Log file (locking it) and writing a new line with a prefix
    inline static QTextStream& acquireLog(const QString & aAcquirerName);
    inline static void releaseLog(); //!< End a log line and release (unlock) the log file

    static bool encrypt(QString & aStr, ushort aMultiplier = LENGTH_MULTIPLIER); //!< encrypt QString using MyCrypt class
    static bool decrypt(QString & aStr, ushort aMultiplier = LENGTH_MULTIPLIER); //!< decrypt QString using MyCrypt class
    static bool encrypt(std::string & aStr, ushort aMultiplier = LENGTH_MULTIPLIER); //!< encrypt std::string using MyCrypt class
    static bool decrypt(std::string & aStr, ushort aMultiplier = LENGTH_MULTIPLIER); //!< decrypt std::string using MyCrypt class

    inline static void log(const QString &aMessage); //!< Add a log line with message
    inline static void log(const char * aMessage);   //!< Add a log line with message
    inline static void log(const QString & aClassPrefix, const QString &aMessage); //!< Add a log line with prefix then message
    inline static void log(const QString & aClassPrefix, const char * aMessage); //!< Add a log line with prefix then message

public slots:
    void threadDeleted(); //!< Slot in main Thread to close an Session Thread (connected to &QThread::destroyed)

// Singleton pattern
private:
    explicit NntpProxy(QObject *parent = 0); //!< Private constructor to follow singleton pattern
    NntpProxy(const NntpProxy &)              = delete;
    NntpProxy(const NntpProxy &&)             = delete;
    NntpProxy & operator=(const NntpProxy &)  = delete;
    NntpProxy & operator=(const NntpProxy &&) = delete;

    ~NntpProxy(); //!< Destructor

private:
    static NntpProxy  *sInstance;             //!< Proxy instance (singleton)
    static bool        isAcceptingConnection; //!< is the proxy accepting input connections?

    SessionManager    *iSessionMgr; //!< Input Session Manager (holds but NOT owns all the SessionHandlers)
    UserManager       *iUserMgr;    //!< User Manager (holds and owns all the connected Users)
    NntpServerManager *iNntpSrvMgr; //!< NntpServer Manager (holds and owns all the active NntpServers)
    Database          *iDatabase;   //!< Shared Thread-Safe Database Connection


    static MyCrypt   *sCrypt;       //!< Encryption utility
    static ushort     iPortNntp;    //!< Server port (from config file, default 119 for unencrypted service)
    static ushort     iPortMonitor; //!< Monitoring/Control server port (TODO, implementation of MonitoringServer)

    static ushort     iSocketTimeout; //!< Socket Timeout (TODO, add a timer on sockets)

    static ushort     sMaxConnectionsPerUser; //!< max number of connection per user (from config file)

    static Log      *sLogMain;  //!< Log file handler (file name and path from config file)
    static LOG_LEVEL sLogLevel; //!< Log level (TODO TO_USE? from config file)

    static bool      sClientSSL;  //!< Are the clients using SSL (from config file)
    static bool      sMonitoring; //!< Are we using the Monitoring Server

    static QVector<NntpServerParameters *> iServParams; //!< Nntp Servers parameters (parsed from config file)
    static DatabaseParameters             *iDbParams;   //!< Database parameters (parsed from config file)

protected:
    //! QTcpServer, create a SessionHandler via SessionManager and move it to a new Thread
    void incomingConnection(qintptr aSocketDescriptor);


private:
    static bool parseConfig(const QString & aFileName); //!< config file parsing (fill the statics attributes)
    static void _log(const QString &     aMessage); //!< add a line in the log
    static void _log(const char*         aMessage); //!< add a line in the log
};

bool  NntpProxy::isClientSSL(){return NntpProxy::sClientSSL;}

ushort NntpProxy::getMaxConnectionsPerUser(){return NntpProxy::sMaxConnectionsPerUser;}

LOG_LEVEL NntpProxy::logLevel(){return sLogLevel;}

QTextStream & NntpProxy::acquireLog(const char * aAcquirerName){
    QTextStream & is = sLogMain->lockWithNewLine();
    return (is << aAcquirerName);
}
QTextStream & NntpProxy::acquireLog(const QString & aAcquirerName){
    QTextStream & is = sLogMain->lockWithNewLine();
    return (is << aAcquirerName);
}
void NntpProxy::releaseLog(){sLogMain->unlockEndLine();}

void NntpProxy::log(const QString &aMessage){*sLogMain << aMessage;}
void NntpProxy::log(const char * aMessage){*sLogMain << aMessage;}

void NntpProxy::log(const QString & aClassPrefix, const QString &aMessage){
    QString str(aClassPrefix);
    str.append(aMessage);
    *sLogMain << str;
}
void NntpProxy::log(const QString & aClassPrefix, const char * aMessage){
    QString str(aClassPrefix);
    str.append(aMessage);
    *sLogMain << str;
}

#endif // NNTPPROXY_H
