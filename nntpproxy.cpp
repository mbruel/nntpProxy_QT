#include "nntpproxy.h"
#include "sessionmanager.h"
#include "sessionhandler.h"
#include "mythread.h"
#include "usermanager.h"
#include "database.h"
#include "nntpservermanager.h"

#include <QXmlStreamReader>
#include <QFile>
#include <QDate>

ushort NntpProxy::iPortNntp               = cDefaultPortNntp;
ushort NntpProxy::iPortMonitor            = cDefaultPortMonitor;
ushort NntpProxy::iSocketTimeout          = cDefaultSocketTimeout;
ushort NntpProxy::sMaxConnectionsPerUser  = cDefaultMaxConPerUser;

bool NntpProxy::sClientSSL                = cIsClientSSL;
bool NntpProxy::sMonitoring               = cUseMonitorServer;

NntpProxy * NntpProxy::sInstance          = Q_NULLPTR;
bool NntpProxy::isAcceptingConnection     = false;

Log *NntpProxy::sLogMain                  = Q_NULLPTR;
LOG_LEVEL NntpProxy::sLogLevel            = cDefaultLogLevel;

MyCrypt * NntpProxy::sCrypt               = Q_NULLPTR;

QVector<NntpServerParameters *> NntpProxy::iServParams      = QVector<NntpServerParameters *>();
DatabaseParameters             *NntpProxy::iDbParams        = Q_NULLPTR;



NntpProxy *NntpProxy::getInstance(QObject *parent){
    if (sInstance == Q_NULLPTR)
        sInstance = new NntpProxy(parent);

    return sInstance;
}

void NntpProxy::shutDown(){
    _log("shutting down...");

    isAcceptingConnection = false;

    delete sInstance;

    _log("Instance deleted...");
    delete sCrypt;
    delete sLogMain;
    std::cout << "Log deleted...\n";

    delete iDbParams;
    for (int i=0; i<iServParams.size(); ++i){
        delete iServParams[i];
    }

    std::cout << "Shutdown done...\n";
}


NntpProxy::NntpProxy(QObject *parent):
    QTcpServer(parent), iSessionMgr(Q_NULLPTR), iUserMgr(Q_NULLPTR),
    iNntpSrvMgr(Q_NULLPTR), iDatabase(Q_NULLPTR)
{}

bool NntpProxy::initStatics(char * aConfigFile){
    const char * configFile = (aConfigFile!=NULL)?aConfigFile:cConfigFile;
    std::cout << "Config file: " << configFile << "\n";

    if (! parseConfig(configFile) ){
        std::cerr << "Error Parsing config file...\n";
        return false;
    }

    sCrypt = new MyCrypt(cEncryptionKey);

    Nntp::initMaps();

    std::cout << *iDbParams;

    for (int i=0; i<iServParams.size(); ++i){
        std::cout << *(iServParams[i]);
    }

    NntpProxy::sLogMain = new Log(QDate::currentDate().toString("NntpProxy.yyyy.MM"));
    if (!NntpProxy::sLogMain->open()) {
        std::cerr << "Can't open main log file... ("
                  << Log::sPath << "/" << "QTlog.txt\n";
        return false;
    }

    _log("Starting Log!");
    return true;
}

bool NntpProxy::init(char * aConfigFile){
    if (!initStatics(aConfigFile))
        return false;

    iUserMgr    = new UserManager();
    iDatabase   = new Database();

    if (! iDatabase->addDatabase(iDbParams) ){
        std::cerr << "Error adding the Database...\n";
        return false;
    }

    if (!iDatabase->connect()){
        _log("Error connectiong to the Database...");
        return false;
    }


    iNntpSrvMgr = new NntpServerManager(iServParams, *iUserMgr);
    if (!iNntpSrvMgr->canConnectToNntpServers()){
        _log("Error connecting to some NntpServer...");
        return false;
    }

    iSessionMgr = new SessionManager(*iUserMgr, *iDatabase, *iNntpSrvMgr);

    return true;
}

bool NntpProxy::startProxy(char * aConfigFile){
    if (!init(aConfigFile))
        return false;

    QString str;
    if (!this->listen(QHostAddress::Any, iPortNntp)){
        str = "Server can't listen on port ";
    } else {
        isAcceptingConnection = true;
        str = "Server started, listening on port: ";
    }

    str += QString::number(iPortNntp);
    _log(str);

    return isAcceptingConnection;
}


NntpProxy::~NntpProxy()
{
    std::cout << "destructor\n";

    _log("Deleting NntpProxy!");
    delete iSessionMgr;
    delete iNntpSrvMgr;
    delete iUserMgr;
    delete iDatabase;

}

void NntpProxy::incomingConnection(qintptr aSocketDescriptor){

    // We have a new connection
    QString str("New incoming connection: ");
    str += QString::number(aSocketDescriptor);
    _log(str);

    if (!isAcceptingConnection){
        _log("Error, the proxy is not accepting connections");
        return;
    }

    MyThread * thread = new MyThread(aSocketDescriptor);
    thread->start(); // start the event loop

    // Create the SessionHandler handler
    SessionHandler *session = iSessionMgr->newSession(aSocketDescriptor, thread);
    session->moveToThread(thread);

    connect(session, &SessionHandler::destroyed, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(thread, &QThread::destroyed, this, &NntpProxy::threadDeleted);

    emit session->startConnection(); // starting the connection inside the new thread
}

void NntpProxy::threadDeleted(){
    _log("Thread deleted");
}

bool NntpProxy::encrypt(QString & aStr, ushort aMultiplier){
    std::string enc;
    bool ret = sCrypt->enc(aStr.toStdString(), enc, aMultiplier);
    if (ret)
        aStr = QString::fromStdString(enc);
    return ret;
}

bool NntpProxy::encrypt(std::string & aStr, ushort aMultiplier){
    std::string enc;
    bool ret = sCrypt->enc(aStr, enc, aMultiplier);
    if (ret)
        aStr = enc;
    return ret;
}

bool NntpProxy::decrypt(QString & aStr, ushort aMultiplier){
    std::string dec;
    bool ret = sCrypt->dec(aStr.toStdString(), dec, aMultiplier);
    if (ret)
        aStr = QString::fromStdString(dec);
    return ret;
}

bool NntpProxy::decrypt(std::string & aStr, ushort aMultiplier){
    std::string dec;
    bool ret = sCrypt->dec(aStr, dec, aMultiplier);
    if (ret)
        aStr = dec;
    return ret;
}

bool NntpProxy::parseConfig(const QString & aFileName)
{
    QFile file(aFileName);
    if (! file.open(QFile::ReadOnly | QFile::Text) ){
        std::cerr << "Error Opening XML file: " << aFileName;
        return false;
    }

    QXmlStreamReader xml; // xml reader
    xml.setDevice(&file); // Initialise l'instance reader avec le flux XML venant de file

    NntpServerParameters *serv  = Q_NULLPTR;
    bool servSection            = false;
    bool databaseSection        = false;

    while (! xml.atEnd()){
        xml.readNext();

        if (xml.isStartElement()){
            if (xml.name() == "server"){
                serv = new NntpServerParameters();
                servSection     = true;
            }
            else if (xml.name() == "authinfo"){
                serv->auth  = true;
            }
            else if (xml.name() == "database"){
                iDbParams = new DatabaseParameters();
                databaseSection = true;
            } else if (xml.name() == "name") {
                serv->name = xml.readElementText().trimmed();
            } else if (xml.name() == "port") {
                if (servSection)
                    serv->port = xml.readElementText().trimmed().toInt();
                else if (databaseSection)
                    iDbParams->port = xml.readElementText().trimmed().toInt();
                else
                    iPortNntp = xml.readElementText().trimmed().toInt();
            } else if (xml.name() == "portMonitoring") {
                iPortMonitor = xml.readElementText().trimmed().toInt();
            } else if (xml.name() == "socketTimeout") {
                iSocketTimeout = xml.readElementText().trimmed().toInt();
            } else if (xml.name() == "maxConnections") {
                serv->maxConnections = xml.readElementText().trimmed().toInt();
            } else if (xml.name() == "ssl") {
                if (xml.readElementText().trimmed().toLower() == "yes")
                    serv->ssl = true;
                else
                    serv->ssl = false;
            } else if (xml.name() == "logFolder") {
                Log::sPath = xml.readElementText().trimmed();
            } else if (xml.name() == "type") {
                iDbParams->type = xml.readElementText().trimmed();
            } else if (xml.name() == "qtDriver") {
                iDbParams->driver = xml.readElementText().trimmed();
            } else if (xml.name() == "host") {
                iDbParams->host = xml.readElementText().trimmed();
            } else if (xml.name() == "login") {
                if (servSection)
                    serv->login = xml.readElementText().trimmed();
                else if (databaseSection)
                    iDbParams->login = xml.readElementText().trimmed();
            } else if (xml.name() == "pass") {
                if (servSection)
                    serv->pass = xml.readElementText().trimmed();
                else if (databaseSection)
                    iDbParams->pass = xml.readElementText().trimmed();
            } else if (xml.name() == "dbName") {
                iDbParams->name = xml.readElementText().trimmed();
            } else if (xml.name() == "maxUserConnections") {
                sMaxConnectionsPerUser = xml.readElementText().trimmed().toInt();
            } else if (xml.name() == "clientSSL") {
                if (xml.readElementText().trimmed().toLower() == "yes")
                    NntpProxy::sClientSSL = true;
            } else if (xml.name() == "monitoring") {
                if (xml.readElementText().trimmed().toLower() == "yes")
                    NntpProxy::sMonitoring = true;
            }
        }

        else if(xml.isEndElement()){
            if (xml.name() == "server") {
                servSection = false;
                iServParams.append(serv);
            } else if (xml.name() == "database") {
                databaseSection = false;
            }
        }

    }

    if (xml.hasError()){
        std::cerr << "Error Parsing: " << xml.errorString();
        file.close();
        return false;
    }

    file.close();
    return true;
}


void NntpProxy::_log(const QString &     aMessage) {
    QString str("[NntpProxy] ");
    str += aMessage;
    *sLogMain << str;
}

void NntpProxy::_log(const char*         aMessage) {
    QString str("[NntpProxy] ");
    str += aMessage;
    *sLogMain << str;
}

std::ostream &  operator<<(std::ostream &stream, const QString &str) {
   stream << str.toStdString();
   return stream;
}

std::ostream & operator<<(std::ostream &stream, const NntpServerParameters & p) {

    stream << "\t<server>\n"
           << "\t\t<name>" << p.name << "</name>\n"
           << "\t\t<port>" << p.port << "</port>\n";
    if (p.auth){
        stream << "\t\t<authinfo>\n"
               << "\t\t\t<login>" << p.login << "</login>\n"
               << "\t\t\t<pass>"  << p.pass  << "</pass>\n"
               << "\t\t</authinfo>\n";
    }

    stream << "\t\t<maxConnections>" << p.maxConnections << "</maxConnections>\n"
           << "\t\t<ssl>" << p.ssl << "</ssl>\n"
           << "\t</server>\n";

    return stream;
}

std::ostream & operator<<(std::ostream &stream, const DatabaseParameters & p) {
    stream << "\t<database>\n"
           << "\t\t<type>"   << p.type   << "</type>\n"
           << "\t\t<qtDriver>" << p.driver << "</qtDriver>\n"
           << "\t\t<host>"   << p.host   << "</host>\n"
           << "\t\t<port>"   << p.port   << "</port>\n"
           << "\t\t<login>"  << p.login  << "</login>\n"
           << "\t\t<pass>****</pass>\n"
           << "\t\t<name>" << p.name << "</name>\n"
           << "\t</database>\n";

    return stream;
}
