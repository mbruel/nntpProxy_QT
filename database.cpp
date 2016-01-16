#include "database.h"
#include "user.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDate>
#include <QMutexLocker>

Database::Database():
    mMutex(), iLogPrefix("[Database] ")
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
        bool ret;
        ushort nbTry = 0;
        do {
            ret = iDb.open();
            if (!ret){
                _log_error("connecting to the DB", iDb.lastError());
#ifdef LOG_DATABASE_ACTIONS
                _log("[connect] Let's try to close the database and reOpen...");
#endif
                iDb.close();
            }
        } while (!ret && (nbTry++ < cDatabaseConnectionTry));

#ifdef LOG_DATABASE_ACTIONS
        if (ret)
            _log("Database connected");
#endif
        return ret;
    }
}

bool Database::prepareSqlRequest(QSqlQuery &aQuery, const char * aSqlReq){
    bool ret;
    ushort nbTry = 0;
    do{
        ret = aQuery.prepare(aSqlReq);
        if (!ret){
            _log_error("preparing request", aQuery.lastError());

            // Error #2006, MySQL server has gone away QMYSQL3: Unable to prepare statement
            if (aQuery.lastError().number() == cMysqlConnectionTimeout){
#ifdef LOG_DATABASE_ACTIONS
                _log("[prepareSqlRequest] MySql Timeout, let's close the connection and reopen it");
#endif
                iDb.close();
                connect();

                // Closing the DB invalidate all QSqlQuery, we need to recreate it
                aQuery = QSqlQuery();
            }
        }
    } while (!ret && (nbTry++ < 1));

    return ret;
}

bool Database::checkAuthentication(User *const aUser, const QString aPass){
    QMutexLocker lock(&mMutex);

    if (!connect())
        return false;

    QSqlQuery qCheckAuthentication;
    bool ret = prepareSqlRequest(qCheckAuthentication, cSqlCheckAuthentication);
    if (!ret){
        _log_error("preparing request qCheckAuthentication", qCheckAuthentication.lastError());
        qCheckAuthentication.finish();
        return ret;
    }

    qCheckAuthentication.bindValue(":login", aUser->getLogin());
    qCheckAuthentication.bindValue(":pass", aPass);

    ret = qCheckAuthentication.exec();
    if (!ret){
        _log_error("executing request qCheckAuthentication", qCheckAuthentication.lastError());
        qCheckAuthentication.finish();
        return ret;
    }


    // If no record, wrong Authentication
    if (!qCheckAuthentication.next()){
        QTextStream &ostream = NntpProxy::acquireLog(iLogPrefix);
        ostream << "There are no records for this user/pass: ("
                << aUser->getLogin() << " : " << aPass << ")";
        NntpProxy::releaseLog();
        qCheckAuthentication.finish();
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
    if (!prepareSqlRequest(callStored, cSqlAddUserSize)) {
        QTextStream &ostream = NntpProxy::acquireLog(iLogPrefix);
        QSqlError err = callStored.lastError();
        ostream << "Error #" << err.number()
                << ", preparing addUserSize(user: " << aUser->getLogin()
                << " (dbId: " << aUser->getDbId() << ") "
                << "ip: " << aUser->getIp()
                << ", download size: " << size
                << ", month: " << theMonth
                << "): " << err.text();
        NntpProxy::releaseLog();
        callStored.finish();
        return 0;
    }

    callStored.bindValue(":p_user_id", aUser->getDbId(), QSql::In);
    callStored.bindValue(":p_month", theMonth, QSql::In);
    callStored.bindValue(":p_ip", aUser->getIp(), QSql::In);
    callStored.bindValue(":p_size", size, QSql::In);

    if(!callStored.exec()) {
        QTextStream &ostream = NntpProxy::acquireLog(iLogPrefix);
        QSqlError err = callStored.lastError();
        ostream << "Error #" << err.number()
                << ", executing addUserSize(user: " << aUser->getLogin()
                << " (dbId: " << aUser->getDbId() << ") "
                << "ip: " << aUser->getIp()
                << ", download size: " << size
                << ", month: " << theMonth
                << "): " << err.text();
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
