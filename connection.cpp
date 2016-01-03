#include "connection.h"
#include "nntpproxy.h"

#include <QByteArray>
#include <QSslSocket>
#include <QSslKey>
#include <QSslCertificate>
#include <QFile>
#include <QAbstractSocket>


Connection::Connection(qintptr aSocketDescriptor, bool ssl, bool servSocket, const char * aClassName):
    QObject(), iSocketDescriptor(aSocketDescriptor), isSsl(ssl),
    isServerSocket(servSocket), iSocket(Q_NULLPTR), iLogPrefix(aClassName)
{
    iLogPrefix.append("[").append(QString::number(iSocketDescriptor)).append("] ");

#ifdef LOG_CONSTRUCTORS
    QTextStream &is = NntpProxy::acquireLog(iLogPrefix);
    is << "Constructor, isSsl: " << isSsl
       << ", isServerSocket: " << isServerSocket;
    NntpProxy::releaseLog();
#endif
}


Connection::~Connection(){
#ifdef LOG_CONSTRUCTORS
    _log("Destructor (deleting iSocket)");
#endif
    if (iSocket && iSocket->isOpen())
        iSocket->disconnectFromHost();

    delete iSocket;
}


bool Connection::startTcpConnection(const char* aHost, ushort aPort){

    _log("Starting connection...");

    if (isSsl) {
        if (!createSslSocket())
            return false;
    } else
        iSocket = new QTcpSocket();



    // if server side socket we attach it to the socketDescriptor
    if (isServerSocket){
        // Attach the socket to the native descriptor
        if(!iSocket->setSocketDescriptor(iSocketDescriptor)) {
            QString err("Setting socket Descriptor: ");
            err += iSocket->errorString();
#ifdef LOG_CONNECTION_ERRORS_BEFORE_EMIT_SIGNALS
            _log(err);
#endif
            emit socketError(err);
            return false;
        }
    } else {
        iSocket->connectToHost(aHost, aPort);
        // we need to wait...
        if(!iSocket->waitForConnected(5000)){
            QString err("connecting to host: ");
            err += aHost;
            err += ":";
            err += QString::number(aPort);
            err += ". socket error: ";
            err += iSocket->errorString();
#ifdef LOG_CONNECTION_ERRORS_BEFORE_EMIT_SIGNALS
            _log(err);
#endif
            emit socketError(err);
            return false;
        }
    }

    // connect socket and signal
    // note - Qt::DirectConnection is used because it's multithreaded
    //        This makes the slot to be invoked immediately, when the signal is emitted.

    qRegisterMetaType<QAbstractSocket::SocketError>("SocketError" );
    connect(iSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(iSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                    this, SLOT(onErrors(QAbstractSocket::SocketError)), Qt::QueuedConnection);


    if (isSsl){
        QSslSocket *ssl_socket = static_cast<QSslSocket*>(iSocket);
        if (isServerSocket)
            ssl_socket->startServerEncryption();
        else
            ssl_socket->startClientEncryption();
#ifdef LOG_SSL_STEPS
        _log("> Encryption handshake done");
#endif
    }


    // If Server socket send Hello Message and wait for requests
    if (isServerSocket){
        iSocket->write(Nntp::getResponse(201));
    } else {
        do {
            iSocket->waitForReadyRead();
        } while (!iSocket->canReadLine());
        QByteArray lineArr = iSocket->readLine();

        if(strncmp(lineArr.constData(), Nntp::getResponse(200), 3) != 0){
            QString err("Reading welcome message. Should start with 200... Server message: ");
            err += lineArr.constData();
#ifdef LOG_CONNECTION_ERRORS_BEFORE_EMIT_SIGNALS
            _log(err);
#endif
            emit socketError(err);
            return false;
        }
    }

    _log("> Client Connected");

    emit connected();
    return true;
}

void Connection::startAsyncRead(){
    _log("startAsyncRead");
    connect(iSocket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
}

void Connection::closeConnection(){
    _log("closeConnection");
    disconnect(iSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}


bool Connection::createSslSocket(){
    _log("SSL socket");

    QSslSocket *ssl_socket = new QSslSocket();
    iSocket = ssl_socket;


    connect(iSocket, SIGNAL(encrypted()), this, SLOT(onEncryptedSocket()));
    connect(iSocket, SIGNAL(sslErrors(QList<QSslError>)),
                    this, SLOT(onSslErrors(QList<QSslError>)));

    ssl_socket->setProtocol(QSsl::TlsV1_2);

    QByteArray key;
    QFile KeyFile(cSslPrivateKey);
    if(KeyFile.open(QIODevice::ReadOnly)) {
        key = KeyFile.readAll();
        KeyFile.close();
    } else {
        QString err("Opening SSL key: ");
        err += KeyFile.errorString();
#ifdef LOG_CONNECTION_ERRORS_BEFORE_EMIT_SIGNALS
        _log(err);
#endif
        emit socketError(err);
        return false;
    }

    QSslKey sslKey(key, QSsl::Rsa);
    ssl_socket->setPrivateKey(sslKey);

    // Load server ssl certificate from file
    QByteArray cert;
    QFile CertFile(cSslCertificate);
    if(CertFile.open(QIODevice::ReadOnly)) {
        cert = CertFile.readAll();
        CertFile.close();
    } else {
        QString err("Opening SSL certificate: ");
        err += CertFile.errorString();
#ifdef LOG_CONNECTION_ERRORS_BEFORE_EMIT_SIGNALS
        _log(err);
#endif
        emit socketError(err);
        return false;
    }


    QSslCertificate sslCert(cert);
    ssl_socket->setLocalCertificate(sslCert);

    return true;
}


void Connection::onEncryptedSocket(){
#ifdef LOG_SSL_STEPS
    _log("> Socket Encrypted");
#endif
}

void Connection::onSslErrors(const QList<QSslError> &errors){
    QString err("Error SSL Socket: ");
    for(int i=0;i<errors.size();i++)
            err += errors[i].errorString() + "\n";
#ifdef LOG_CONNECTION_ERRORS_BEFORE_EMIT_SIGNALS
    _log(err);
#endif
    emit socketError(err);
}

void Connection::onErrors(QAbstractSocket::SocketError) {
    QString err("Error Socket: ");
    err += iSocket->errorString();
#ifdef LOG_CONNECTION_ERRORS_BEFORE_EMIT_SIGNALS
    _log(err);
#endif
    emit socketError(err);
}


