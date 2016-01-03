#ifndef LOG_H
#define LOG_H

#include <QFile>
#include <QTextStream>
#include <QMutex>

/*!
 * \brief Thread Safe wrapper to write log file. New lines start with the date/time and the thread_id
 */
class Log
{
public:
    static QString sPath; //!< Path of the log file

public:
    explicit Log(const QString & aFileName); //!< constructor with log file name
    Log(const Log &)              = delete;
    Log(const Log &&)             = delete;
    Log & operator=(const Log &)  = delete;
    Log & operator=(const Log &&) = delete;

    ~Log();      //!< close the file handler

    bool open(); //!< Try to open the log file for writing


    //!< Lock the file, create a new line, write the prefix and return the TextStream
    QTextStream & lockWithNewLine();

    //!< end the line and unlock the file (to be used after lockWithNewLine)
    void unlockEndLine();

    Log & operator<<(const QString & aStr); //!< write a new line in the log
    Log & operator<<(const char* aStr);     //!< write a new line in the log

private:
    inline void writeLinePrefix(); //!< write line prefix (date/time plus thread id)

private:
    const QString  iFileName; //!< log file name
    QFile          iFile;     //!< file handler
    QTextStream    iStream;   //!< text output stream
    QMutex         iMutex;    //!< mutex to be thread safe
};

#endif // LOG_H
