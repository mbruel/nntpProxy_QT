#ifndef DATABASE_H
#define DATABASE_H

#include "constants.h"
#include "nntpproxy.h" // inline log functions

QT_FORWARD_DECLARE_CLASS(User)

#include <QSqlDatabase>
#include <QSqlError>
#include <QMutex>

/*!
 * \brief Interface to the Database. Thread safe.
 */
class Database
{
public:
    explicit Database(); //!< default constructor
    Database(const Database &)              = delete;
    Database(const Database &&)             = delete;
    Database & operator=(const Database &)  = delete;
    Database & operator=(const Database &&) = delete;

    ~Database(); //!< destructor will close the connection to the DB

    /*!
     * \brief Add the database to the system
     * \param aDbParam
     * \return if the connection is valid (not open)
     */
    bool addDatabase(DatabaseParameters * const aDbParam);

    bool connect(); //!< Try to connect to the Database

    /*!
     * \brief Check user authentication in the auth table
     * \param aUser: to get the login and fill the rest of the structure
     * \param aPass: encrypted pass
     * \return result from the Database
     */
    bool checkAuthentication(User *const aUser, const QString aPass);

    uint addUserSize(User *aUser); //!< call stored proc add_user_size_QT

private:
    inline void _log(const QString & aMessage) const; //!< log function for QString
    inline void _log(const char    * aMessage) const; //!< log function for char *

private:
    QSqlDatabase      iDb;              //!< actual Database connection
    QMutex            mMutex;           //!< Mutex to be thread safe
    ushort            iConnectionTrial; //!< Number of trial to connect to the DB
    const QString     iLogPrefix;       //!< log prefix

};

void Database::_log(const char* aMessage) const {NntpProxy::log(iLogPrefix, aMessage);}
void Database::_log(const QString & aMessage) const {NntpProxy::log(iLogPrefix, aMessage);}
#endif // DATABASE_H
