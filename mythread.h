#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>

/*!
 * \brief Child of QThread to hold a connection Id and log construction/destruction
 */
class MyThread : public QThread
{
    Q_OBJECT

public:
    MyThread(qintptr aIdSocket); //!< hold the socket Id

    MyThread(const MyThread &)              = delete;
    MyThread(const MyThread &&)             = delete;
    MyThread & operator=(const MyThread &)  = delete;
    MyThread & operator=(const MyThread &&) = delete;

    ~MyThread(); //!< trace destruction

private:
    qintptr iIdSocket; //!< socket id of the input connection

    void _log(const char *aMessage); //!< write trace with connection id
};

#endif // MYTHREAD_H
