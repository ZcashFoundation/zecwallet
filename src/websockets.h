#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H

#include "precompiled.h"

#include "mainwindow.h"
#include "ui_mobileappconnector.h"


QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class WSServer;

// We're going to wrap the websocket in this class, because the underlying QWebSocket might get closed
// or deleted while a callback is waiting to get the data back. Therefore, we write a custom "sendTextMessage"
// class that checks all this before sending.
class ClientWebSocket {
public:
    ClientWebSocket(QWebSocket* c, WSServer* s = nullptr) { client = c; server = s; }

    void sendTextMessage(QString m);
    void close(QWebSocketProtocol::CloseCode code, const QString& msg) { client->close(code, msg); }
private:
    QWebSocket* client;
    WSServer*   server;
};

class WSServer : public QObject
{
    Q_OBJECT
public:
    explicit WSServer(quint16 port, bool debug = false, QObject *parent = nullptr);
    bool isValidConnection(QWebSocket* c) { return m_clients.contains(c); }
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

class WormholeClient : public QObject {
    Q_OBJECT 

private Q_SLOTS:
    void onConnected();
    void onTextMessageReceived(QString message);
    void closed();

public:
    WormholeClient(MainWindow* parent, QString wormholeCode);
    ~WormholeClient();

    void connect();
    void retryConnect();

private:
    MainWindow* parent = nullptr;    
    QWebSocket*  m_webSocket = nullptr;
    
    QTimer* timer            = nullptr;

    QString     code;
    int  retryCount          = 0;
    bool shuttingDown        = false;
};

enum NonceType {
    LOCAL = 1,
    REMOTE
};

enum AppConnectionType {
    DIRECT = 1,
    INTERNET
};

class AppDataServer {
public:
    static AppDataServer* getInstance() {
        if (instance == nullptr) {
            instance = new AppDataServer();
        }
        return instance;
    }

    void          connectAppDialog(MainWindow* parent);
    void          updateConnectedUI();
    void          updateUIWithNewQRCode(MainWindow* mainwindow);

    void          processSendTx(QJsonObject sendTx, MainWindow* mainwindow, std::shared_ptr<ClientWebSocket> pClient);
    void          processMessage(QString message, MainWindow* mainWindow, std::shared_ptr<ClientWebSocket> pClient, AppConnectionType connType);
    void          processGetInfo(QJsonObject jobj, MainWindow* mainWindow, std::shared_ptr<ClientWebSocket> pClient);
    void          processDecryptedMessage(QString message, MainWindow* mainWindow, std::shared_ptr<ClientWebSocket> pClient);
    void          processGetTransactions(MainWindow* mainWindow, std::shared_ptr<ClientWebSocket> pClient);

    QString       decryptMessage(QJsonDocument msg, QString secretHex, QString lastRemoteNonceHex);
    QString       encryptOutgoing(QString msg);

    QString       getWormholeCode(QString secretHex);
    QString       getSecretHex();
    void          saveNewSecret(QString secretHex);

    void          registerNewTempSecret(QString tmpSecretHex, bool allowInternet, MainWindow* main);

    QString       getNonceHex(NonceType nt);
    void          saveNonceHex(NonceType nt, QString noncehex);

    bool          getAllowInternetConnection();
    void          setAllowInternetConnection(bool allow);

    void          saveLastSeenTime();
    QDateTime     getLastSeenTime();

    void          setConnectedName(QString name);   
    QString       getConnectedName();
    bool          isAppConnected();

    QString       connDesc(AppConnectionType t);

    void               saveLastConnectedOver(AppConnectionType type);
    AppConnectionType  getLastConnectionType();

private:
    AppDataServer() = default;

    static AppDataServer*   instance;
    Ui_MobileAppConnector*  ui;

    QString                 tempSecret;
    WormholeClient*         tempWormholeClient = nullptr;
};

class AppDataModel {
public:
    static AppDataModel* getInstance() {
        if (instance == NULL)
            instance = new AppDataModel();

        return instance;
    }

    double getTBalance()     { return balTransparent;  }
    double getZBalance()     { return balShielded; }
    double getTotalBalance() { return balTotal; }

    void   setBalances(double transparent, double shielded) {
        balTransparent = transparent;
        balShielded = shielded;
        balTotal = balTransparent + balShielded;
    }

private:
    AppDataModel() = default;   // Private, for singleton

    double balTransparent;
    double balShielded;
    double balTotal;

    QString saplingAddress;

    static AppDataModel* instance;
};



#endif // WEBSOCKETS_H