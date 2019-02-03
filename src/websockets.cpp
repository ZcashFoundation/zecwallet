#include "websockets.h"

#include "rpc.h"
#include "settings.h"
#include "ui_mobileappconnector.h"
#include "version.h"

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
        AppDataServer::processMessage(message, m_mainWindow, pClient);
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
    QSettings s;

    return s.value("mobileapp/secret", "").toString();
}

void AppDataServer::saveNewSecret(QString secretHex) {
    QSettings s;
    s.setValue("mobileapp/secret", secretHex);

    s.sync();
}

QString AppDataServer::tempSecret = "";
Ui_MobileAppConnector* AppDataServer::ui = nullptr;

void AppDataServer::connectAppDialog(QWidget* parent) {
    QDialog d(parent);
    ui = new Ui_MobileAppConnector();
    ui->setupUi(&d);
    Settings::saveRestore(&d);

    updateUIWithNewQRCode();
    updateConnectedUI();

    QObject::connect(ui->btnDisconnect, &QPushButton::clicked, [=] () {
        QSettings().setValue("mobileapp/connectedname", "");
        saveNewSecret("");

        updateConnectedUI();
    });
    

    d.exec();

    // Cleanup
    tempSecret = "";
    delete ui;
    ui = nullptr;
}

void AppDataServer::updateUIWithNewQRCode() {
    // Get the address of the localhost
    auto addrList = QNetworkInterface::allAddresses();

    // Find a suitable address
    QString ipv4Addr;
    for (auto addr : addrList) {
        if (addr.isLoopback() || addr.protocol() == QAbstractSocket::IPv6Protocol)
            continue;

        ipv4Addr = addr.toString();
        break;
    }

    if (ipv4Addr.isEmpty())
        return;
    
    QString uri = "ws://" + ipv4Addr + ":8237";

    // Get a new secret
    unsigned char* secretBin = new unsigned char[crypto_secretbox_KEYBYTES];
    randombytes_buf(secretBin, crypto_secretbox_KEYBYTES);
    char* secretHex = new char[crypto_secretbox_KEYBYTES*2 + 1];
    sodium_bin2hex(secretHex, crypto_secretbox_KEYBYTES*2+1, secretBin, crypto_secretbox_KEYBYTES);

    QString secretStr(secretHex);
    tempSecret = secretStr;

    QString codeStr = uri + "," + secretStr;

    ui->qrcode->setQrcodeString(codeStr);
}

void AppDataServer::updateConnectedUI() {
    if (ui == nullptr)
        return;

    auto remoteName = QSettings().value("mobileapp/connectedname", "").toString();
    ui->lblRemoteName->setText(remoteName.isEmpty() ?  "(Not connected to any device)" : remoteName);
    ui->btnDisconnect->setEnabled(!remoteName.isEmpty());
}

QString AppDataServer::getNonceHex(NonceType nt) {
    QSettings s;
    QString hex;
    if (nt == NonceType::LOCAL) {
        // The default local nonce starts from 1, to always keep it odd
        auto defaultLocalNonce = "01" + QString("00").repeated(crypto_secretbox_NONCEBYTES-1);
        hex = s.value("mobileapp/localnonce", defaultLocalNonce).toString();
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
    s.sync();
}

QString AppDataServer::encryptOutgoing(QString msg) {
    if (msg.length() % 256 > 0) {
        msg = msg + QString(" ").repeated(256 - (msg.length() % 256));
    }

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
    sodium_hex2bin(secret, crypto_secretbox_KEYBYTES, getSecretHex().toStdString().c_str(), crypto_secretbox_KEYBYTES*2, 
        NULL, NULL, NULL);

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
    
    delete[] noncebin;
    delete[] newLocalNonce;
    delete[] secret;
    delete[] encrpyted;
    delete[] encryptedHex;

    return json.toJson();
}

QString AppDataServer::decryptMessage(QJsonDocument msg, QString secretHex, bool skipNonceCheck) {
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

    assert(crypto_secretbox_KEYBYTES == crypto_hash_sha256_BYTES);
    if (!skipNonceCheck) {
        if (sodium_compare(lastRemoteBin, noncebin, crypto_secretbox_NONCEBYTES) != -1) {
            // Refuse to accept a lower nonce, return an error
            delete[] lastRemoteBin;
            delete[] noncebin;
            return "error";
        }
    }
    
    unsigned char* secret = new unsigned char[crypto_secretbox_KEYBYTES];
    sodium_hex2bin(secret, crypto_secretbox_KEYBYTES, secretHex.toStdString().c_str(), crypto_secretbox_KEYBYTES*2, 
        NULL, NULL, NULL);

    unsigned char* encrypted = new unsigned char[encryptedhex.length() / 2];
    sodium_hex2bin(encrypted, encryptedhex.length() / 2, encryptedhex.toStdString().c_str(), encryptedhex.length(),
                    NULL, NULL, NULL);

    int decryptedLen = encryptedhex.length() / 2 - crypto_secretbox_MACBYTES;
    unsigned char* decrypted = new unsigned char[decryptedLen];
    int result = crypto_secretbox_open_easy(decrypted, encrypted, encryptedhex.length() / 2, noncebin, secret);

    QString payload;
    if (result == -1) {
        payload = "error";        
    } else {
        // Update the last seen remote hex
        saveNonceHex(NonceType::REMOTE, noncehex);

        char* decryptedStr = new char[decryptedLen + 1];
        sodium_memzero(decryptedStr, decryptedLen + 1);
        memcpy(decryptedStr, decrypted, decryptedLen);

        payload = QString(decryptedStr);

        qDebug() << "Decrypted to: " << payload;
        delete[] decryptedStr;
    }

    delete[] secret;
    delete[] lastRemoteBin;
    delete[] noncebin;
    delete[] encrypted;
    delete[] decrypted;
    
    return payload;
}

void AppDataServer::processMessage(QString message, MainWindow* mainWindow, QWebSocket* pClient) {
    // First, extract the command from the message
    auto msg = QJsonDocument::fromJson(message.toUtf8());

    // First check if the message is encrpted
    if (!msg.object().contains("nonce")) {
        // TODO: Log error?
        return;
    }

    auto decrypted = decryptMessage(msg, getSecretHex());

    // If the decryption failed, maybe this is a new connection, so see if the dialog is open and a 
    // temp secret is in place

    auto replyWithError = [=]() {
        auto r = QJsonDocument(QJsonObject{
                    {"error", "Encryption error"}
            }).toJson();
        pClient->sendTextMessage(r);
        return;
    };

    if (decrypted == "error") {
        // If the dialog is open, then there might be a temporary, new secret key. Attempt to decrypt
        // with that.
        if (!tempSecret.isEmpty()) {
            decrypted = decryptMessage(msg, tempSecret, true);
            if (decrypted == "error") {
                // Oh, well. Just return an error
                replyWithError();
                return;
            }
            else {
                // This is a new connection. So, update the remote nonce (to accept any nonce) and the secret
                saveNewSecret(tempSecret);
                AppDataServer::saveNonceHex(NonceType::REMOTE, QString("00").repeated(24));

                processDecryptedMessage(decrypted, mainWindow, pClient);

                // If the Connection UI is showing, we have to update the UI as well
                if (ui != nullptr) {
                    // Update the connected phone information
                    updateConnectedUI();

                    // Update with a new QR Code for safety, so this secret isn't used by anyone else
                    updateUIWithNewQRCode();
                }
            }
        }
        else {
            replyWithError();
        }
    } else {
        processDecryptedMessage(decrypted, mainWindow, pClient);
    }
}

void AppDataServer::processDecryptedMessage(QString message, MainWindow* mainWindow, QWebSocket* pClient) {
    // First, extract the command from the message
    auto msg = QJsonDocument::fromJson(message.toUtf8());

    if (!msg.object().contains("command")) {
        auto r = QJsonDocument(QJsonObject{
            {"errorCode", -1},
            {"errorMessage", "Unknown JSON format"}
        }).toJson();
        pClient->sendTextMessage(encryptOutgoing(r));
        return;
    }
    
    if (msg.object()["command"] == "getInfo") {
        processGetInfo(msg.object(), mainWindow, pClient);
    }
    else if (msg.object()["command"] == "getTransactions") {
        processGetTransactions(mainWindow, pClient);
    }
    else if (msg.object()["command"] == "sendTx") {
        processSendTx(msg.object()["tx"].toObject(), mainWindow, pClient);
    }
    else {
        auto r = QJsonDocument(QJsonObject{
            {"errorCode", -1},
            {"errorMessage", "Command not found:" + msg.object()["command"].toString()}
        }).toJson();
        pClient->sendTextMessage(encryptOutgoing(r));
    }
}

void AppDataServer::processSendTx(QJsonObject sendTx, MainWindow* mainwindow, QWebSocket* pClient) {
    auto error = [=](QString reason) {
        auto r = QJsonDocument(QJsonObject{
           {"errorCode", -1},
           {"errorMessage", "Couldn't send Tx:" + reason}
        }).toJson();
        pClient->sendTextMessage(encryptOutgoing(r));
        return;
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
        // Filter out balances that don't have the requisite amount
        if (allBalances->value(i) < amt)
            continue;

        bals.append(QPair<QString, double>(i, allBalances->value(i)));
    }

    if (bals.isEmpty()) {
        error("No sapling or transparent addresses with enough balance to spend.");
        return;
    }

    std::sort(bals.begin(), bals.end(), [=](const QPair<QString, double>a, const QPair<QString, double> b) -> bool {
        // Sort z addresses first
        return a.first > b.first;
    });

    tx.fromAddr = bals[0].first;
    tx.toAddrs = { ToFields{ sendTx["to"].toString(), amt, sendTx["memo"].toString(), sendTx["memo"].toString().toUtf8().toHex()} };

    // TODO: Respect the autoshield change setting

    QString validation = mainwindow->doSendTxValidations(tx);
    if (!validation.isEmpty()) {
        error(validation);
        return;
    }

    json params = json::array();
    mainwindow->getRPC()->fillTxJsonParams(params, tx);
    std::cout << std::setw(2) << params << std::endl;

    // And send the Tx
    mainwindow->getRPC()->executeTransaction(tx,
        [=] (QString opid) {

        }, 
        [=] (QString opid, QString txid) {
            auto r = QJsonDocument(QJsonObject{
               {"version", 1.0},
               {"command", "sendTxSubmitted"},
               {"txid",  txid}
            }).toJson();
            if (pClient->isValid())
                pClient->sendTextMessage(encryptOutgoing(r));
        },
        [=] (QString opid, QString errStr) {
            auto r = QJsonDocument(QJsonObject{
               {"version", 1.0},
               {"command", "sendTxFailed"},
               {"err",  errStr}
            }).toJson();
            if (pClient->isValid())
                pClient->sendTextMessage(encryptOutgoing(r));
        }   
    );

    auto r = QJsonDocument(QJsonObject{
            {"version", 1.0},
            {"command", "sendTx"},
            {"result",  "success"}
        }).toJson();
    pClient->sendTextMessage(encryptOutgoing(r));
}

void AppDataServer::processGetInfo(QJsonObject jobj, MainWindow* mainWindow, QWebSocket* pClient) {
    auto connectedName = jobj["name"].toString();

    
    if (mainWindow == nullptr || mainWindow->getRPC() == nullptr ||
            mainWindow->getRPC()->getAllBalances() == nullptr) {
        pClient->close(QWebSocketProtocol::CloseCodeNormal, "Not yet ready");
        return;
    }


    // Max spendable safely from a z address and from any address
    double maxZSpendable = 0;
    double maxSpendable = 0;
    for (auto a : mainWindow->getRPC()->getAllBalances()->keys()) {
        if (Settings::getInstance()->isSaplingAddress(a)) {
            if (mainWindow->getRPC()->getAllBalances()->value(a) > maxZSpendable) {
                maxZSpendable = mainWindow->getRPC()->getAllBalances()->value(a);
            }
        }
        if (mainWindow->getRPC()->getAllBalances()->value(a) > maxSpendable) {
            maxSpendable = mainWindow->getRPC()->getAllBalances()->value(a);
        }
    }

    {
        QSettings s;
        s.setValue("mobileapp/connectedname", connectedName);
    }

    auto r = QJsonDocument(QJsonObject{
        {"version", 1.0},
        {"command", "getInfo"},
        {"saplingAddress", mainWindow->getRPC()->getDefaultSaplingAddress()},
        {"tAddress", mainWindow->getRPC()->getDefaultTAddress()},
        {"balance", AppDataModel::getInstance()->getTotalBalance()},
        {"maxspendable", maxSpendable},
        {"maxzspendable", maxZSpendable},
        {"tokenName", Settings::getTokenName()},
        {"zecprice", Settings::getInstance()->getZECPrice()},
        {"serverversion", QString(APP_VERSION)}
    }).toJson();
    pClient->sendTextMessage(encryptOutgoing(r));
}

void AppDataServer::processGetTransactions(MainWindow* mainWindow, QWebSocket* pClient) {
    QJsonArray txns;
    auto model = mainWindow->getRPC()->getTransactionsModel();

    // Manually add pending ops, so that computing transactions will also show up
    auto wtxns = mainWindow->getRPC()->getWatchingTxns();
    for (auto opid : wtxns.keys()) {
        txns.append(QJsonObject{
            {"type", "send"},
            {"datetime", QDateTime::currentSecsSinceEpoch()},
            {"amount", Settings::getDecimalString(wtxns[opid].tx.toAddrs[0].amount)},
            {"txid", ""},
            {"address", wtxns[opid].tx.toAddrs[0].addr},
            {"memo", wtxns[opid].tx.toAddrs[0].txtMemo},
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

    auto r = QJsonDocument(QJsonObject{
            {"version", 1.0},
            {"command", "getTransactions"},
            {"transactions", txns}
        }).toJson();
    pClient->sendTextMessage(encryptOutgoing(r));
}

// ==============================
// AppDataModel
// ==============================
AppDataModel* AppDataModel::instance = NULL;
