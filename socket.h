#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>
#include <QWebSocket>
#include <QAbstractSocket>
#include <QCoreApplication>
#include <QTimer>
#include <QOperatingSystemVersion>
#include <QFile>
#include <QRegExp>
#include <QJsonObject>
#include <QJsonDocument>
#include <QProcess>
#include <QDateTime>

class Socket : public QObject
{
    Q_OBJECT
public:
    explicit Socket(QCoreApplication *app, QObject *parent = nullptr);

signals:

public slots:
    void onSocketConnected();
    void onSocketClosed();
    void initSocket();
    void timeoutReached();
    void onTextMessageReceived(const QString &msg);
    void sendPing();
    void resetSocketBridgeAttempts();
    void errorWhileRestartingBridge(QProcess::ProcessError error);

private:
    void loadSettings();
    QMap<QString, QVariant> retrieveJsonDoc(const QString &message);
    QWebSocket *m_webSocket;
    QString m_url;                  //
    QCoreApplication *m_app;       //app pointer, that help to self close the program
    QTimer *m_ttlTimer;           //time to live timer for keep alive program
    QTimer *m_recursiveTimer;    //recursive timer for ping check
    int m_triesCount;           //number of times we try to reach the server before to send a restart signal to the socketbridge
    int m_triesThreshold;      //the threshold choosen for m_triesCount
    QString m_serviceName;
    QProcess *m_process;
    int m_recursiveTime;    //the time of a ping

};

#endif // SOCKET_H
