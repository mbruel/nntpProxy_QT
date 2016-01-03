#include "user.h"
#include "nntpproxy.h"
//#include <algorithm> // std::sort

#include <QDateTime>

User::User(const QString & aIpAddress, const QString & aLogin):
    iIpAddress(aIpAddress), iStartTimeMs(QDateTime::currentMSecsSinceEpoch()), iId(0),
    iLogin(aLogin), isBlocked(0), iNumInputCons(), iNumNntpCons(),
    mNumInput(), mNumNntp(), iDownloadSize(0), mDownSize(), iNntpServCons()
{
#ifdef LOG_CONSTRUCTORS
    QTextStream &ostream = NntpProxy::acquireLog("[User] ");
    ostream << "Constructor: " << *this;
    NntpProxy::releaseLog();
#endif
}

User::~User(){
#ifdef LOG_CONSTRUCTORS
    QTextStream &ostream = NntpProxy::acquireLog("[User] ");
    ostream << "Destructor: " << *this;
    NntpProxy::releaseLog();
#endif
}

bool User::operator== (const User & aUser) const{
    if (aUser.iLogin != iLogin)
        return false;
    if (aUser.iIpAddress != iIpAddress)
        return false;

    return true;
}

QTextStream &  operator<<(QTextStream & stream, const User &aUser){
    QMutexLocker lock1(&aUser.mNumInput);
    QMutexLocker lock2(&aUser.mNumNntp);

    stream << "User #" << aUser.iId << ": "
           << aUser.iLogin << "@" << aUser.iIpAddress
           << " (in: "  << aUser.iNumInputCons
           << ", out: " << aUser.iNumNntpCons
           << ") ";

    for (auto it = aUser.iNntpServCons.cbegin(); it != aUser.iNntpServCons.cend(); ++it){
        stream << "{serv: " << it->first
               << ", cons: " << it->second
               << "} ";
    }

    stream << aUser.downSize();

    return stream;
}


QString User::str() const{
    QMutexLocker lock1(&mNumInput);
    QMutexLocker lock2(&mNumNntp);

    QString str("User #");
    str += QString::number(iId);
    str += ": ";
    str += iLogin;
    str += "@";
    str += iIpAddress;
    str += " (in: ";
    str += QString::number(iNumInputCons);
    str += ", out: ";
    str += QString::number(iNumNntpCons);
    str += ") ";

    for (auto it = iNntpServCons.cbegin(); it != iNntpServCons.cend(); ++it){
        str += "{serv: ";
        str += QString::number(it->first);
        str += ". cons: ";
        str += QString::number(it->second);
        str += "} ";
    }

    return str;
}


QString User::downSize() const{

    mDownSize.lock();
    float  size      = iDownloadSize;
    float  ko        = size / 1024;
    qint64 endTimeMs = QDateTime::currentMSecsSinceEpoch();
    mDownSize.unlock();

    QString format("Bytes");
    if (ko >= 1){
        format = "kB";
        size   = ko;

        if (size >= 1024){
            format = "MB";
            size /= 1024;

            if (size >= 1024){
                format = "GB";
                size /= 1024;
            }
        }
    }

    qint64 millis = endTimeMs - iStartTimeMs;
    qint64 second    = (millis / 1000) % 60;
    qint64 minute    = (millis / 60000) % 60;
    qint64 hour      = (millis / 3600000) % 24;

    float debit = 0;
    if (millis > 0)
        debit = 1000.0*ko/millis;

    return QString("%1 %2 in %3:%4:%5 (%6 kB/s)").arg(
                QString::number(size, 'f', 2), format, QString::number(hour),
                QString::number(minute), QString::number(second), QString::number(debit, 'f', 0));
}

void User::newNntpConnection(ushort aServerId){
    QMutexLocker lock(&mNumNntp);
    auto it = iNntpServCons.find(aServerId);
    if (it == iNntpServCons.end()){
        iNntpServCons[aServerId] = 1;
    } else {
        ++(it->second);
    }
    ++iNumNntpCons;
}

bool User::delNntpConnection(ushort aServerId){
    QMutexLocker lock(&mNumNntp);
    auto it = iNntpServCons.find(aServerId);
    if (it != iNntpServCons.end()){
        if (it->second == 1) {
            iNntpServCons.erase(it);
        } else {
            --(it->second);
        }
        --iNumNntpCons;
        return true;
    }
    return false;
}

bool compNntpServConsByValue(const std::pair<ushort,ushort>& lhs, const std::pair<ushort,ushort>& rhs) {
    return lhs.second < rhs.second;
}

vectNntpSrvOrderByCons User::getVectorOfNntpServerOrderedByNumberOfConnections_noLock() const{
    vectNntpSrvOrderByCons servers(iNntpServCons.cbegin(), iNntpServCons.cend());
    std::sort(servers.begin(), servers.end(), compNntpServConsByValue);
    return servers;
}
