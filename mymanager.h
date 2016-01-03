#ifndef MYMANAGER_H
#define MYMANAGER_H

#include "constants.h"
#include "nntpproxy.h"

#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>

/*!
 * \brief "Thread Safe" wrapper of a QList of pointers
 */
template<typename Data> class MyManager
{
public:
    explicit MyManager(const char * aDataType); //!< \param name of the data type
    MyManager(const MyManager &)              = delete;
    MyManager(const MyManager &&)             = delete;
    MyManager & operator=(const MyManager &)  = delete;
    MyManager & operator=(const MyManager &&) = delete;

    virtual ~MyManager(); //!< will delete all the elements of iList

    /*!
     * \brief erase data from the list (blocking or non blocking, Thread_Safe by default)
     * \param aData    : data to remove
     * \param useMutex : shall we protect (lock) the list
     * \return
     */
    bool erase(Data *aData, bool useMutex = true);

    inline ushort size() const; //!< return the size of the list Thread_Safe
    inline ushort size_noLock() const; //!< return the size of the list (non blocking)

    void dump() const;               //!< Dump manager in the log file
    void dump(QTextStream &aStream); //!< Dump manager on text stream

protected:
    inline QString getSizeStr_noLock() const; //!< return a string of the size (log purposes)

    /*!
     * \brief find data in the list (blocking or non blocking, Thread_Safe by default)
     * \param aId      : data id (Data type should have a getId() method)
     * \param useMutex : shall we protect (lock) the list
     * \return
     */
    Data * find(ushort aId,  bool useMutex = true) const;

    /*!
     * \brief find data in the list (blocking or non blocking, Thread_Safe by default)
     * \param aData    : data to find (dereference pointer and use operator== on objects)
     * \param useMutex : shall we protect (lock) the list
     * \return
     */
    Data * find(Data *aData, bool useMutex = true) const;

    inline void _log(const char    * aMessage) const; //!< log function for char *
    inline void _log(const QString & aMessage) const; //!< log function for QString

protected:
    QList<Data *>   iList;      //!< list of pointers
    mutable QMutex *mMutex;     //!< Mutex to be thread safe
    const QString   iDataName;  //!< string name of the data type
    const QString   iLogPrefix; //!< log prefix iDataName<Manager>
};

//////////////////////
/// inlines functions
template<typename Data> ushort MyManager<Data>::size() const{
    QMutexLocker lock(mMutex);
    return iList.size();
}
template<typename Data> ushort MyManager<Data>::size_noLock() const{
    return iList.size();
}

template<typename Data> void MyManager<Data>::_log(const QString & aMessage) const{
    NntpProxy::log(iLogPrefix, aMessage);
}

template<typename Data> void MyManager<Data>::_log(const char * aMessage) const{
    NntpProxy::log(iLogPrefix, aMessage);
}

template<typename Data> QString MyManager<Data>::getSizeStr_noLock() const{
    return QString("Current list size: ").append(QString::number(iList.size()));
}


////////////////////
// Normal functions

template<typename Data> MyManager<Data>::MyManager(const char * aDataType):
    iList(), mMutex(new QMutex()), iDataName(aDataType),
    iLogPrefix(QString("[").append(iDataName).append("Manager] "))
{
#ifdef LOG_CONSTRUCTORS
    _log("Constructor");
#endif
}

template<typename Data> MyManager<Data>::~MyManager(){
    mMutex->lock();
#ifdef LOG_CONSTRUCTORS
    QString str("Destructor, ");
    str += getSizeStr_noLock();
    _log(str);
#endif
    for (int i=0; i<iList.size(); ++i)
        delete iList[i];

    iList.clear();
    mMutex->unlock();
    delete mMutex;
}

template<typename Data> bool MyManager<Data>::erase(Data *aData, bool useMutex){
    if (useMutex)
        mMutex->lock();

    bool out = iList.removeOne(aData);

    QTextStream &is = NntpProxy::acquireLog(iLogPrefix);
    is << "erase " << iDataName << " with id: " << aData->getId()
       << ", result: " << out
       << ", " << getSizeStr_noLock();
    NntpProxy::releaseLog();

    if (useMutex)
        mMutex->unlock();

    return out;
}

template<typename Data> Data * MyManager<Data>::find(ushort aId, bool useMutex) const{
    Data *out = Q_NULLPTR;

    if (useMutex)
        mMutex->lock();

    for (int i=0; i<iList.size(); ++i){
        if (iList[i]->getId() == aId){
            out = iList[i];
            break;
        }
    }

    if (useMutex)
        mMutex->unlock();

    return out;
}

template<typename Data> Data * MyManager<Data>::find(Data *aData, bool useMutex) const{
    Data *out = Q_NULLPTR;

    if (useMutex)
        mMutex->lock();

    for (int i=0; i<iList.size(); ++i){
        if (*aData == *(iList[i])){
            out = iList[i];
            break;
        }
    }

    if (useMutex)
        mMutex->unlock();

    return out;
}

template<typename Data> void MyManager<Data>::dump() const{
    QMutexLocker lock(mMutex);
    QTextStream &ostream = NntpProxy::acquireLog(iLogPrefix);
    ostream << "Dump, " << getSizeStr_noLock() << "\n";
    for (int i=0; i<iList.size(); ++i){
        ostream << "\t- " << *(iList[i]) << "\n";
    }
    NntpProxy::releaseLog();
}

template<typename Data> void MyManager<Data>::dump(QTextStream &aStream){
    QMutexLocker lock(mMutex);
    aStream << iLogPrefix << "Dump, " << getSizeStr_noLock() << "\n";
    for (int i=0; i<iList.size(); ++i){
        aStream << "\t- " << *(iList[i]) << "\n";
    }
}

#endif // MYMANAGER_H
