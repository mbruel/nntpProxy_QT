#include <QCoreApplication>
#include "nntpproxy.h"
#include "log.h"
#include <iostream>

#include <QThread>
#include <csignal>

#include <QFile>

void handleShutDown(int signal){
    Q_UNUSED(signal);
    std::cout << "Try to shutdown the proxy\n";
    NntpProxy::shutDown();
    std::cout << "Try to exit the event loop\n";
    qApp->exit(0);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    signal(SIGINT,  &handleShutDown);// shut down on ctrl-c
    signal(SIGTERM, &handleShutDown);// shut down on killall
    signal(SIGQUIT, &handleShutDown);// not sure...

    NntpProxy *theProxy = NntpProxy::getInstance();
//    QObject::connect(app, &QCoreApplication::aboutToQuit, theProxy, &NntpProxy::deleteInstance);


    char * configFile = NULL;
    if (argc == 2){
        configFile = argv[1];
        if (!QFile::exists(configFile)){
            std::cout << "Error the config file doesn't exists...\n";
            return 1;
        }
    }

    if (!theProxy->startProxy(configFile)){
        std::cout << "Error starting Proxy...\n";
        return 1;
    }

    app.exec(); // Run the event loop

    std::cout << "\n\nEvent loop ended...\n\n";
    return 0;
}



