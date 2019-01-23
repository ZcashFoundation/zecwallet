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
    if (m_pWebSocketServer->listen(QHostAddress::AnyIPv4, port)) {
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
    
    if (msg.object()["command"] == "getInfo") {
        return processGetInfo(mainWindow);
    }
    else if (msg.object()["command"] == "getTransactions") {
        return processGetTransactions(mainWindow);
    }
    else if (msg.object()["command"] == "sendTx") {
        return processSendTx(msg.object()["tx"].toObject(), mainWindow);
    }
    else {
        return QJsonDocument(QJsonObject{
            {"errorCode", -1},
            {"errorMessage", "Command not found:" + msg.object()["command"].toString()}
        });
    }
}

QJsonDocument AppDataServer::processSendTx(QJsonObject sendTx, MainWindow* mainwindow) {
    auto error = [=](QString reason) -> QJsonDocument {
        return QJsonDocument(QJsonObject{
           {"errorCode", -1},
           {"errorMessage", "Couldn't send Tx:" + reason}
        });
    };

    // Create a Tx Object
    Tx tx;
    tx.fee = Settings::getMinerFee();

    // Find a from address that has at least the sending amout
    double amt = sendTx["amount"].toString().toDouble();
    auto allBalances = mainwindow->getRPC()->getAllBalances();
    QList<QPair<QString, double>> bals;
    for (auto i : allBalances->keys()) {
        // Filter out sprout Txns
        if (Settings::getInstance()->isSproutAddress(i))
            continue;
        bals.append(QPair<QString, double>(i, allBalances->value(i)));
    }

    if (bals.isEmpty()) {
        return error("No sapling or transparent addresses");
    }

    std::sort(bals.begin(), bals.end(), [=](const QPair<QString, double>a, const QPair<QString, double> b) ->bool {
        // If same type, sort by amount
        if (a.first[0] == b.first[0]) {
            return a.second > b.second;
        }
        else {
            return a > b;
        }
    });

    if (amt > bals[0].second) {
        // There isn't any any address capable of sending the Tx.
        return error("Amount exceeds the balance of your largest address.");
    }

    tx.fromAddr = bals[0].first;
    tx.toAddrs = { ToFields{ sendTx["to"].toString(), amt, sendTx["memo"].toString(), sendTx["memo"].toString().toUtf8().toHex()} };

    // TODO: Respect the autoshield change setting

    QString validation = mainwindow->doSendTxValidations(tx);
    if (!validation.isEmpty()) {
        return error(validation);
    }

    json params = json::array();
    mainwindow->getRPC()->fillTxJsonParams(params, tx);
    std::cout << std::setw(2) << params << std::endl;

    // And send the Tx
    mainwindow->getRPC()->sendZTransaction(params, [=](const json& reply) {
        QString opid = QString::fromStdString(reply.get<json::string_t>());

        // And then start monitoring the transaction
        mainwindow->getRPC()->addNewTxToWatch(tx, opid);

        // TODO: Handle the error if the computed Tx fails.
    });

    return QJsonDocument(QJsonObject{
            {"version", 1.0},
            {"command", "sendTx"},
            {"result",  "success"}
        });
}

QJsonDocument AppDataServer::processGetInfo(MainWindow* mainWindow) {
    return QJsonDocument(QJsonObject{
        {"version", 1.0},
        {"command", "getInfo"},
        {"saplingAddress", mainWindow->getRPC()->getDefaultSaplingAddress()},
        {"tAddress", mainWindow->getRPC()->getDefaultTAddress()},
        {"balance", AppDataModel::getInstance()->getTotalBalance()},
        {"tokenName", Settings::getTokenName()},
        {"zecprice", Settings::getInstance()->getZECPrice()}
    });
}

QJsonDocument AppDataServer::processGetTransactions(MainWindow* mainWindow) {
    QJsonArray txns;
    auto model = mainWindow->getRPC()->getTransactionsModel();

    // Manually add pending ops, so that computing transactions will also show up
    auto wtxns = mainWindow->getRPC()->getWatchingTxns();
    for (auto opid : wtxns.keys()) {
        txns.append(QJsonObject{
            {"type", "send"},
            {"datetime", QDateTime::currentSecsSinceEpoch()},
            {"amount", Settings::getDecimalString(wtxns[opid].toAddrs[0].amount)},
            {"txid", ""},
            {"address", wtxns[opid].toAddrs[0].addr},
            {"memo", wtxns[opid].toAddrs[0].txtMemo},
            {"confirmations", 0}
            });
    }
    
    // Add transactions
    for (int i = 0; i < model->rowCount(QModelIndex()) && i < Settings::getMaxMobileAppTxns(); i++) {
        txns.append(QJsonObject{
            {"type", model->getType(i)},
            {"datetime", model->getDate(i)},
            {"amount", model->getAmt(i)},
            {"txid", model->getTxId(i)},
            {"address", model->getAddr(i)},
            {"memo", model->getMemo(i)},
            {"confirmations", model->getConfirmations(i)}
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