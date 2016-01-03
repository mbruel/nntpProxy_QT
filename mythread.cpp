#include "mythread.h"
#include "nntpproxy.h"

MyThread::MyThread(qintptr aIdSocket):iIdSocket(aIdSocket)
{
#ifdef LOG_CONSTRUCTORS
    _log("Constructor");
#endif
}

MyThread::~MyThread(){
#ifdef LOG_CONSTRUCTORS
     _log("Destructor");
#endif
}

void MyThread::_log(const char *aMessage){
    QString str("MyThread");
    str += "[";
    str += QString::number(iIdSocket);
    str += "] ";
    str += aMessage;

    NntpProxy::log(str);
}
