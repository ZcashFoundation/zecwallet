#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H

#include "mainwindow.h"
#include "precompiled.h"


QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class WSServer : public QObject
{
    Q_OBJECT
public:
    explicit WSServer(quint16 port, bool debug = false, QObject *parent = nullptr);
    ~WSServer();

Q_SIGNALS:
    void closed();

private Q_SLOTS:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();

private:
    QWebSocketServer *m_pWebSocketServer;
    MainWindow *m_mainWindow;
    QList<QWebSocket *> m_clients;
    bool m_debug;
};

#endif // WEBSOCKETS_H