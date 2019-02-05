#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H

#include "precompiled.h"

#include "mainwindow.h"
#include "ui_mobileappconnector.h"


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

enum NonceType {
    LOCAL = 1,
    REMOTE
};

class AppDataServer {
public:
    static AppDataServer* getInstance() {
        if (instance == nullptr) {
            instance = new AppDataServer();
        }
        return instance;
    }

    void          connectAppDialog(QWidget* parent);
    void          updateConnectedUI();
    void          updateUIWithNewQRCode();

    void          processSendTx(QJsonObject sendTx, MainWindow* mainwindow, QWebSocket* pClient);
    void          processMessage(QString message, MainWindow* mainWindow, QWebSocket* pClient);
    void          processGetInfo(QJsonObject jobj, MainWindow* mainWindow, QWebSocket* pClient);
    void          processDecryptedMessage(QString message, MainWindow* mainWindow, QWebSocket* pClient);
    void          processGetTransactions(MainWindow* mainWindow, QWebSocket* pClient);

    QString       decryptMessage(QJsonDocument msg, QString secretHex, bool skipNonceCheck = false);
    QString       encryptOutgoing(QString msg);

    QString       getSecretHex();
    void          saveNewSecret(QString secretHex);

    QString       getNonceHex(NonceType nt);
    void          saveNonceHex(NonceType nt, QString noncehex);

private:
    AppDataServer() = default;

    static AppDataServer*   instance;
    QString                 tempSecret;
    Ui_MobileAppConnector*  ui;
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