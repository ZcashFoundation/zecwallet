
#include "websockets.h"
#include "rpc.h"
#include "settings.h"

WSServer::WSServer(quint16 port, bool debug, QObject *parent) :
    QObject(parent),
    m_pWebSocketServer(new QWebSocketServer(QStringLiteral("Direct Connection Server"),
                                            QWebSocketServer::NonSecureMode, this)),
    m_debug(debug)
{
    m_mainWindow = (MainWindow *) parent;
    if (m_pWebSocketServer->listen(QHostAddress::LocalHost, port)) {
        if (m_debug)
            qDebug() << "Echoserver listening on port" << port;
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
                this, &WSServer::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &WSServer::closed);
    }
}

WSServer::~WSServer()
{
    qDebug() << "Closing websocket";
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

void WSServer::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &WSServer::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &WSServer::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &WSServer::socketDisconnected);

    m_clients << pSocket;
}

void WSServer::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "Message received:" << message;
    if (pClient) {
        auto json = AppDataServer::processMessage(message, m_mainWindow);        
        pClient->sendTextMessage(json.toJson());
    }
}

void WSServer::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "Binary Message received:" << message;
    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}

void WSServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "socketDisconnected:" << pClient;
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

// ==============================
// AppDataServer
// ==============================
QJsonDocument AppDataServer::processMessage(QString message, MainWindow* mainWindow) {
    // First, extract the command from the message
    auto msg = QJsonDocument::fromJson(message.toUtf8());
    if (!msg.object().contains("command")) {
        return QJsonDocument(QJsonObject{
            {"errorCode", -1},
            {"errorMessage", "Unknown JSON format"}
        });
    }
    
    if (msg["command"] == "getInfo") {
        return processGetInfo(mainWindow);
    }
    else if (msg["command"] == "getTransactions") {
        return processGetTransactions(mainWindow);
    }
    else {
        return QJsonDocument(QJsonObject{
            {"errorCode", -1},
            {"errorMessage", "Command not found:" + msg["command"].toString()}
        });
    }
}

QJsonDocument AppDataServer::processGetInfo(MainWindow* mainWindow) {
    return QJsonDocument(QJsonObject{
        {"version", 1.0},
        {"command", "getInfo"},
        {"saplingAddress", mainWindow->getRPC()->getDefaultSaplingAddress()},
        {"balance", AppDataModel::getInstance()->getTotalBalance()},
        {"zecprice", Settings::getInstance()->getZECPrice()}
    });
}

QJsonDocument AppDataServer::processGetTransactions(MainWindow* mainWindow) {
    QJsonArray txns;
    auto model = mainWindow->getRPC()->getTransactionsModel();
    for (int i = 0; i < model->rowCount(QModelIndex()); i++) {
        txns.append(QJsonObject{
            {"type", model->getType(i)},
            {"datetime", model->getDate(i)},
            {"amount", model->getAmt(i)}
        });
    }

    return QJsonDocument(QJsonObject{
            {"version", 1.0},
            {"command", "getTransactions"},
            {"transactions", txns}
        });
}

// ==============================
// AppDataModel
// ==============================
AppDataModel* AppDataModel::instance = NULL;