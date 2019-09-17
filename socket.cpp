#include "socket.h"

Socket::Socket(QCoreApplication *app, QObject *parent) :
    QObject(parent),
    m_url("http://localhost:4559"),
    m_app(app),
    m_triesCount(0),
    m_triesThreshold(3),
    m_serviceName("dati_socket")

{
    loadSettings();
    m_webSocket = new QWebSocket();
    m_ttlTimer = new QTimer(this);
    m_recursiveTimer = new QTimer(this);
    m_process = new QProcess(this);
    m_process->setProgram("systemctl");
    m_process->setArguments(QStringList() << "restart" << m_serviceName);

    connect(m_process, &QProcess::errorOccurred, this, &Socket::errorWhileRestartingBridge);
    connect(m_webSocket, &QWebSocket::connected, this, &Socket::onSocketConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &Socket::onSocketClosed);
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
          [=](QAbstractSocket::SocketError error){
        qDebug() << "Error emitted: \n errorcode:" << error << " \n error msg: " << m_webSocket->errorString();
        if(!m_webSocket->isValid()){
            if(m_triesCount < m_triesThreshold){//nous ne sommes pas encore à la limite des essais
                m_recursiveTimer->stop();
                m_recursiveTimer->singleShot(5000, m_webSocket, SLOT(open));
                m_triesCount++;
            }else{//nous avons atteint la limite. Nous relançons le socketbridge
                m_process->start();
                m_process->waitForFinished();                       //Waits for execution to complete

                QString StdOut = m_process->readAllStandardOutput();  //Reads standard output
                QString StdError = m_process->readAllStandardError();   //Reads standard error

                qDebug() <<  "\n Printing the standard output..........\n"
                          <<  StdOut
                          << "\n Printing the standard error..........\n"
                          << StdError << "\n\n";
            }
        }
    });
    m_webSocket->open(QUrl(m_url));

    //any 1hour= 3600000ms, we throw the timer so that the process is killed
    m_ttlTimer->singleShot((3600000), this, SLOT(timeoutReached()));
}

void Socket::errorWhileRestartingBridge(QProcess::ProcessError error){
    qDebug() << "failed to restart socketbridge. Error code: " << error;
}

void Socket::loadSettings(){
    QString fileName("");
    if(QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows){
        fileName = "params.txt";
    }else {
        fileName = "/usr/local/params.txt";
    }

    QFile configFile(fileName);
    if(!configFile.open(QIODevice::ReadOnly)){
        qDebug() << "Oops a key file is missing (" << fileName << "). keepalive will use defaults values";
        return;
    }

    QMap<QString, QString> params=QMap<QString, QString>();
    QTextStream in(&configFile);
    while (!in.atEnd())
    {
       QString line = in.readLine();
       QStringList lineParts = line.split(":");
       lineParts.removeAt(0);
       params.insert(line.split(":").at(0), lineParts.join(":").trimmed());
    }
    configFile.close();

    if(!params.isEmpty()){
       if(params.contains("keepalive_socketbridge_service_name")){
            qDebug() << "retrieved keepalive_socketbridge_service_name from " << fileName << " /value: "<< params.value("keepalive_socketbridge_service_name");
            m_serviceName = params.value("keepalive_socketbridge_service_name");
        }

       if(params.contains("keepalive_attemps_threshold")){
           qDebug() << "retrieved keepalive_threshold from " << fileName << " /value: "<< params.value("keepalive_attemps_threshold");
           m_triesThreshold = params.value("keepalive_attemps_threshold").toInt();
       }

       if(params.contains("keepalive_socket_address")){
           qDebug() << "retrieved keepalive_socket_address from " << fileName << " /value: "<< params.value("keepalive_socket_address");
           m_url = params.value("keepalive_socket_address");
       }
    }
}

void Socket::onSocketClosed(){
    m_recursiveTimer->stop();
    m_webSocket->open(m_url);
}

void Socket::onSocketConnected(){
    qDebug() << "WebSocket connected";
    connect(m_webSocket, &QWebSocket::textMessageReceived,this, &Socket::onTextMessageReceived);
    sendPing();
}

QMap<QString, QVariant> Socket::retrieveJsonDoc(const QString &message)
{
    QByteArray json_bytes = message.toLocal8Bit();
    auto json_doc = QJsonDocument::fromJson(json_bytes);

    if(json_doc.isNull() | !json_doc.isObject() | json_doc.isEmpty() ){
        qDebug()<< "Fail to create JSON doc. message******** \n" << message << "\n********** was not a valid JSON";
        return QMap<QString, QVariant>();
    }

    return json_doc.toVariant().toMap();
}

void Socket::onTextMessageReceived(const QString &msg){
    QMap<QString, QVariant> msgMap = retrieveJsonDoc(msg);

    if(msgMap.contains("pong")){
       m_triesCount = 0;
       m_recursiveTimer->stop();
       m_recursiveTimer->singleShot(10000, this, SLOT(sendPing()));
    }
}
void Socket::sendPing(){
    m_webSocket->sendTextMessage("{\"type\": \"ping\", \"clientsockettype\":\"KEEP_CLIENT\"}");
}
void Socket::timeoutReached(){
     m_app->exit(1);
}