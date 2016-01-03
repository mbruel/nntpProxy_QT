#include "database.h"
#include "user.h"
#include <QSqlQuery>
#include <QDate>
#include <QMutexLocker>

Database::Database():
    mMutex(), iConnectionTrial(0), iLogPrefix("[Database] ")
{
#ifdef LOG_CONSTRUCTORS
    _log("Constructor");
#endif
}

Database::~Database(){
#ifdef LOG_CONSTRUCTORS
    _log("Destructor");
#endif
    if (iDb.isOpen())
        iDb.close();
}

bool Database::addDatabase(DatabaseParameters * const aDbParam){
    if (QSqlDatabase::contains()){
        _log("addDatabase: DB already exists...");
        iDb = QSqlDatabase::database();
    } else {
        iDb = QSqlDatabase::addDatabase(aDbParam->driver);
        iDb.setHostName(aDbParam->host);
        iDb.setPort(aDbParam->port);

        QString userName(aDbParam->login);
        if (!NntpProxy::decrypt(userName)){
            _log("Error decrypting username...");
            return false;
        }
        iDb.setUserName(userName);

        QString pass(aDbParam->pass);
        if (!NntpProxy::decrypt(pass)){
            _log("Error decrypting password...");
            return false;
        }
        iDb.setPassword(pass);

        iDb.setDatabaseName(aDbParam->name);
        _log("addDatabase: DB added!");
    }

    return iDb.isValid();
}


bool Database::connect(){

    if(iDb.isOpen()){
#ifdef LOG_DATABASE_ACTIONS
        _log("Database already connected");
#endif
        return true;
    }
    else{

        bool ret = iDb.open();
        if (!ret){
            _log(QString("Error connecting to DB: ").append(iDb.lastError().text()) );
            if (iConnectionTrial < cDatabaseConnectionTry){
                _log("Let's try again...");
                ++iConnectionTrial;
                iDb = QSqlDatabase::database();
                connect();
            }

        } else {
#ifdef LOG_DATABASE_ACTIONS
            _log("Database connected");
#endif
        }
        return ret;
    }
}


bool Database::checkAuthentication(User *const aUser, const QString aPass){
    QMutexLocker lock(&mMutex);

    if (!connect())
        return false;

    QSqlQuery qCheckAuthentication;
    bool ret = qCheckAuthentication.prepare(
                "select id, blocked from auth "
                "where (login = :login) and (pass = :pass)");

    if (!ret){
        _log(QString("Error preparing request...").append(qCheckAuthentication.lastError().text()));
        return ret;
    }

    qCheckAuthentication.bindValue(":login", aUser->getLogin());
    qCheckAuthentication.bindValue(":pass", aPass);

    ret = qCheckAuthentication.exec();
    if (!ret){
        _log(QString("Error preparing request...").append(qCheckAuthentication.lastError().text()));
        return ret;
    }


    // If no record, wrong Authentication
    if (!qCheckAuthentication.next()){
        QTextStream &ostream = NntpProxy::acquireLog(iLogPrefix);
        ostream << "There are no records for this user/pass: ("
                << aUser->getLogin() << " : " << aPass << ")";
        NntpProxy::releaseLog();
        return false;
    }

    // We've a match, let's fill aUser
    aUser->setDbId(qCheckAuthentication.value(0).toInt());
    aUser->setBlocked(qCheckAuthentication.value(1).toBool());

//    qCheckAuthentication.clear();
    qCheckAuthentication.finish();

    _log("Authentication OK!!!");
    return true;
}

uint Database::addUserSize(User *const aUser){
    QMutexLocker lock(&mMutex);

    if (!connect())
        return 0;

    QSqlQuery callStored;

    QString theMonth(QDate::currentDate().toString("yyyy.MM"));
    int size = aUser->getDownloadedSize()/1048576; // in MB

    // Out parameter code is MySQL specific
    callStored.prepare("call add_user_size_QT(:p_user_id, :p_month, :p_ip, :p_size, @m_size)");

    callStored.bindValue(":p_user_id", aUser->getDbId(), QSql::In);
    callStored.bindValue(":p_month", theMonth, QSql::In);
    callStored.bindValue(":p_ip", aUser->getIp(), QSql::In);
    callStored.bindValue(":p_size", size, QSql::InOut);

    if(!callStored.exec()) {
        QTextStream &ostream = NntpProxy::acquireLog(iLogPrefix);
        ostream << "Error addUserSize(user: " << aUser->getLogin()
                << " (dbId: " << aUser->getDbId() << ") "
                << "ip: " << aUser->getIp()
                << ", download size: " << size
                << ", month: " << theMonth
                << "): " << callStored.lastError().text();
        NntpProxy::releaseLog();
        callStored.finish();
        return 0;
    }
    callStored.exec("select @m_size");
    callStored.next();
    uint monthSize = callStored.value(0).toInt();

    callStored.finish();

    QTextStream &ostream = NntpProxy::acquireLog(iLogPrefix);
    ostream << "addUserSize(user: " << aUser->getLogin()
            << " (dbId: " << aUser->getDbId() << ") "
            << "ip: " << aUser->getIp()
            << ", download size: " << size
            << ", month: " << theMonth
            << ") => new month size = " << monthSize;
    NntpProxy::releaseLog();


    return monthSize;
}
