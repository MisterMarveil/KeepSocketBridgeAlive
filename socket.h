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

class Socket : public QObject
{
    Q_OBJECT
public:
    explicit Socket(QCoreApplication *app, QObject *parent = nullptr);

signals:

public slots:
    void onSocketConnected();
    void onSocketClosed();
    void timeoutReached();
    void onTextMessageReceived(const QString &msg);
    void sendPing();
    void errorWhileRestartingBridge(QProcess::ProcessError error);

private:
    void loadSettings();
    QMap<QString, QVariant> retrieveJsonDoc(const QString &message);
    QWebSocket *m_webSocket;
    QString m_url;
    QCoreApplication *m_app;
    QTimer *m_ttlTimer;
    QTimer *m_recursiveTimer;
    int m_triesCount;
    int m_triesThreshold;
    QString m_serviceName;
    QProcess *m_process;

};

#endif // SOCKET_H
