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
        if (!json.isEmpty())      
            pClient->sendTextMessage(AppDataServer::encryptOutgoing(json.toJson()));
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
QString AppDataServer::getSecretHex() {
    return "secret";
}

QString AppDataServer::getNonceHex(NonceType nt) {
    QSettings s;
    QString hex;
    if (nt == NonceType::LOCAL) {
        hex = s.value("mobileapp/localnonce", QString("00").repeated(crypto_secretbox_NONCEBYTES)).toString();
    }
    else {
        hex = s.value("mobileapp/remotenonce", QString("00").repeated(crypto_secretbox_NONCEBYTES)).toString();
    }
    return hex;
}

void AppDataServer::saveNonceHex(NonceType nt, QString noncehex) {
    QSettings s;
    assert(noncehex.length() == crypto_secretbox_NONCEBYTES * 2);
    if (nt == NonceType::LOCAL) {
        s.setValue("mobileapp/localnonce", noncehex);
    }
    else {
        s.setValue("mobileapp/remotenonce", noncehex);
    }
}

QString AppDataServer::encryptOutgoing(QString msg) {
    QString localNonceHex = getNonceHex(NonceType::LOCAL);

    unsigned char* noncebin = new unsigned char[crypto_secretbox_NONCEBYTES];
    sodium_hex2bin(noncebin, crypto_secretbox_NONCEBYTES, localNonceHex.toStdString().c_str(), localNonceHex.length(),
        NULL, NULL, NULL);

    // Increment the nonce +2 and save
    sodium_increment(noncebin, crypto_secretbox_NONCEBYTES);
    sodium_increment(noncebin, crypto_secretbox_NONCEBYTES);

    char* newLocalNonce = new char[crypto_secretbox_NONCEBYTES*2 + 1];
    sodium_memzero(newLocalNonce, crypto_secretbox_NONCEBYTES*2 + 1);
    sodium_bin2hex(newLocalNonce, crypto_secretbox_NONCEBYTES*2+1, noncebin, crypto_box_NONCEBYTES);

    qDebug() << "Local nonce was " << localNonceHex << " now is " << QString(newLocalNonce);
    saveNonceHex(NonceType::LOCAL, QString(newLocalNonce));

    unsigned char* secret = new unsigned char[crypto_secretbox_KEYBYTES];
    crypto_hash_sha256(secret, (const unsigned char*)"secret", QString("secret").length());

    int msgSize = strlen(msg.toStdString().c_str());
    unsigned char* encrpyted = new unsigned char[ msgSize + crypto_secretbox_MACBYTES];

    crypto_secretbox_easy(encrpyted, (const unsigned char *)msg.toStdString().c_str(), msgSize, noncebin, secret);

    int encryptedHexSize = (msgSize + crypto_secretbox_MACBYTES) * 2 + 1;
    char * encryptedHex = new char[encryptedHexSize];     
    sodium_memzero(encryptedHex, encryptedHexSize);
    sodium_bin2hex(encryptedHex, encryptedHexSize, encrpyted, msgSize + crypto_secretbox_MACBYTES);

    qDebug() << "Encrypted to " << QString(encryptedHex);

    auto json =  QJsonDocument(QJsonObject{
            {"nonce", QString(newLocalNonce)},
            {"payload", QString(encryptedHex)}
        });
    
    return json.toJson();
}

QString AppDataServer::decryptMessage(QJsonDocument msg) {
    // Decrypt and then process
    QString noncehex = msg.object().value("nonce").toString();
    QString encryptedhex = msg.object().value("payload").toString();

    // Check to make sure that the nonce is greater than the last known remote nonce
    QString lastRemoteHex = getNonceHex(NonceType::REMOTE);
    unsigned char* lastRemoteBin = new unsigned char[crypto_secretbox_NONCEBYTES];
    sodium_hex2bin(lastRemoteBin, crypto_secretbox_NONCEBYTES, lastRemoteHex.toStdString().c_str(), lastRemoteHex.length(),
        NULL, NULL, NULL);

    unsigned char* noncebin = new unsigned char[crypto_secretbox_NONCEBYTES];
    sodium_hex2bin(noncebin, crypto_secretbox_NONCEBYTES, noncehex.toStdString().c_str(), noncehex.length(),
        NULL, NULL, NULL);

    assert(sodium_compare(lastRemoteBin, noncebin, crypto_secretbox_NONCEBYTES) == -1);
    assert(crypto_secretbox_KEYBYTES == crypto_hash_sha256_BYTES);

    // Update the last seem remote hex
    saveNonceHex(NonceType::REMOTE, noncehex);

    unsigned char* secret = new unsigned char[crypto_secretbox_KEYBYTES];
    crypto_hash_sha256(secret, (const unsigned char*)"secret", QString("secret").length());

    unsigned char* encrypted = new unsigned char[encryptedhex.length() / 2];
    sodium_hex2bin(encrypted, encryptedhex.length() / 2, encryptedhex.toStdString().c_str(), encryptedhex.length(),
                    NULL, NULL, NULL);

    int decryptedLen = encryptedhex.length() / 2 - crypto_secretbox_MACBYTES;
    unsigned char* decrypted = new unsigned char[decryptedLen];
    crypto_secretbox_open_easy(decrypted, encrypted, encryptedhex.length() / 2, noncebin, secret);

    char* decryptedStr = new char[decryptedLen + 1];
    sodium_memzero(decryptedStr, decryptedLen + 1);
    memcpy(decryptedStr, decrypted, decryptedLen);

    QString payload = QString(decryptedStr);

    qDebug() << "Decrypted to: " << payload;

    delete[] secret;
    delete[] lastRemoteBin;
    delete[] noncebin;
    delete[] encrypted;
    delete[] decrypted;
    delete[] decryptedStr;
    
    return payload;
}

QJsonDocument AppDataServer::processMessage(QString message, MainWindow* mainWindow) {
    // First, extract the command from the message
    auto msg = QJsonDocument::fromJson(message.toUtf8());

    // First check if the message is encrpted
    if (msg.object().contains("nonce")) {
        return processMessage(decryptMessage(msg), mainWindow);
    }

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
