#include "log.h"
#include <QMutexLocker>
#include <QThread>
#include <QDateTime>

QString Log::sPath;

Log::Log(const QString & aFileName):
    iFileName(aFileName), iFile(Log::sPath+"/"+aFileName), iMutex()
{}


Log::~Log(){
    if (iFile.isOpen())
        iFile.close();
}


bool Log::open(){
    QMutexLocker lock(&iMutex);

    if (iFile.isOpen())
        return true;

    if (!iFile.open(QIODevice::Append | QIODevice::Text))
        return false;
    if (!iFile.isWritable())
        return false;

    iStream.setDevice(&iFile);

    return true;
}


void Log::writeLinePrefix(){
    iStream << "[" << QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss")
            << "] [Thread " << QThread::currentThreadId() << "] ";
}


QTextStream & Log::lockWithNewLine(){
    iMutex.lock();
    writeLinePrefix();
    return iStream;
}


void Log::unlockEndLine(){
    iStream << "\n";
    iStream.flush();
    iMutex.unlock();
}


Log & Log::operator<< (const QString & aStr){
    QMutexLocker lock(&iMutex);
    writeLinePrefix();
    iStream << aStr << "\n";
    iStream.flush();
    return *this;
}


Log & Log::operator<< (const char* aStr){
    QMutexLocker lock(&iMutex);
    writeLinePrefix();
    iStream << aStr << "\n";
    iStream.flush();
    return *this;
}
