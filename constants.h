#ifndef NNTPSERVERPARAMETERS
#define NNTPSERVERPARAMETERS

#include <iostream>
#include <QString>

#include <QtGlobal> // ushort...

#define LOG_CONSTRUCTORS 1
#define LOG_CONNECTION_ERRORS_BEFORE_EMIT_SIGNALS 1

//#define LOG_SSL_STEPS 1

//#define LOG_DATABASE_ACTIONS 1

#define LOG_INPUT_DATA   1
//#define LOG_INPUT_AUTH   1

//#define LOG_NEWS_DATA    1
//#define LOG_NEWS_AUTH    1

static const constexpr char* cEncryptionKey  = "my secret encryption key";

static const constexpr char* cConfigFile     = "./config.xml";

// openssl req -x509 -newkey rsa:1024 -keyout key.pem -out cert.pem -days 3650 -nodes
static const constexpr char* cSslPrivateKey  = "./key.pem";
static const constexpr char* cSslCertificate = "./cert.pem";

enum LOG_LEVEL {LOG_ALL = 0, LOG_MEDIUM_TRACE=4, LOG_SHORT_TRACE=9};

static const ushort    cAuthenticationTry       = 5;
static const ushort    cDatabaseConnectionTry   = 3;
static const ushort    cMysqlConnectionTimeout  = 2006;

static const LOG_LEVEL cDefaultLogLevel      = LOG_ALL;
static const ushort    cDefaultPortNntp      = 119;
static const ushort    cDefaultPortMonitor   = 1111;
static const ushort    cDefaultSocketTimeout = 5000;
static const ushort    cDefaultMaxConPerUser = 3;
static const bool      cIsClientSSL          = false;
static const bool      cUseMonitorServer     = false;

static const constexpr char* cSqlCheckAuthentication =
        "select id, blocked from auth where (login = :login) and (pass = :pass);";

static const constexpr char* cSqlAddUserSize         =
        "call add_user_size_QT(:p_user_id, :p_month, :p_ip, :p_size, @m_size);";


struct NntpServerParameters{
    QString name;
    ushort  port;
    bool    auth;
    QString login;
    QString pass;
    ushort  maxConnections;
    bool    ssl;

    NntpServerParameters():
       name(""), port(119), auth(false), login(""), pass(""), maxConnections(1), ssl(false)
    {}

    NntpServerParameters(const char * aName, ushort aPort = 119, bool aAuth = false,
                         const char * aLogin = "", const char *aPass = "",
                         ushort aMaxCon = 1, bool aSsl = false):
       name(aName), port(aPort), auth(aAuth), login(aLogin),
       pass(aPass), maxConnections(aMaxCon), ssl(aSsl)
    {}

    NntpServerParameters(const NntpServerParameters& aParams):
        name(aParams.name), port(aParams.port), auth(aParams.auth), login(aParams.login),
        pass(aParams.pass), maxConnections(aParams.maxConnections), ssl(aParams.ssl)
    {}

    NntpServerParameters(NntpServerParameters&& aParams):
        name(std::move(aParams.name)), port(aParams.port), auth(aParams.auth), login(std::move(aParams.login)),
        pass(std::move(aParams.pass)), maxConnections(aParams.maxConnections), ssl(aParams.ssl)
    {}

};

struct DatabaseParameters{
    QString type;
    QString driver;
    QString host;
    ushort  port;
    QString login;
    QString pass;
    QString name;

    DatabaseParameters() = default;

    DatabaseParameters(const DatabaseParameters& aParams):
        type(aParams.type), driver(aParams.driver),
        host(aParams.host), port(aParams.port), login(aParams.login),
        pass(aParams.pass), name(aParams.name)
    {}

    DatabaseParameters(const char * aDriver, const char * aHost, ushort aPort,
                       const char * aLogin, const char * aPass, const char *aName):
        type(), driver(aDriver), host(aHost), port(aPort), login(aLogin),
        pass(aPass), name(aName)
    {}

    DatabaseParameters(DatabaseParameters&& aParams):
        type(std::move(aParams.type)), driver(std::move(aParams.driver)),
        host(std::move(aParams.host)), port(aParams.port), login(std::move(aParams.login)),
        pass(std::move(aParams.pass)), name(std::move(aParams.name))
    {}
};

std::ostream &  operator<<(std::ostream &stream, const QString &str);

std::ostream & operator<<(std::ostream &stream, const NntpServerParameters & p);

std::ostream & operator<<(std::ostream &stream, const DatabaseParameters & p);

#endif // NNTPSERVERPARAMETERS

