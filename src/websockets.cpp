#include "websockets.h"

#include "controller.h"
#include "settings.h"
#include "ui_mobileappconnector.h"
#include "version.h"

// Weap the sendTextMessage to check if the connection is valid and that the parent WebServer didn't close this connection
// for some reason.
void ClientWebSocket::sendTextMessage(QString m) {
    if (client) {
        if (server && !server->isValidConnection(client)) {
            return;
        }

        if (client->isValid())
            client->sendTextMessage(m);
    }
}

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
        std::shared_ptr<ClientWebSocket> client = std::make_shared<ClientWebSocket>(pClient, this);
        AppDataServer::getInstance()->processMessage(message, m_mainWindow, client, AppConnectionType::DIRECT);
    }
}

void WSServer::processBinaryMessage(QByteArray message)
{
    //QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "Binary Message received:" << message;

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

//===============================
// WormholeClient
//===============================
WormholeClient::WormholeClient(MainWindow* p, QString wormholeCode) {
    this->parent = p;
    this->code = wormholeCode;
    connect();
}

WormholeClient::~WormholeClient() {
    shuttingDown = true;

    if (m_webSocket->isValid()) {
        m_webSocket->close();
    }

    if (timer)
        timer->stop();
        
    delete timer;
}

void WormholeClient::connect() {
    delete m_webSocket;
    m_webSocket = new QWebSocket();

    QObject::connect(m_webSocket, &QWebSocket::connected, this, &WormholeClient::onConnected);
    QObject::connect(m_webSocket, &QWebSocket::disconnected, this, &WormholeClient::closed);

    m_webSocket->open(QUrl("wss://wormhole.zecqtwallet.com:443"));
    //m_webSocket->open(QUrl("ws://127.0.0.1:7070"));
}

void WormholeClient::retryConnect() {    
    QTimer::singleShot(5 * 1000 * pow(2, retryCount), [=]() {
        if (retryCount < 10) {
            qDebug() << "Retrying websocket connection";
            this->retryCount++;
            connect();
        }
        else {
            qDebug() << "Retry count exceeded, will not attempt retry any more";
        }
    });
}

// Called when the websocket is closed. If this was closed without our explicitly closing it, 
// then we need to try and reconnect
void WormholeClient::closed() {
    if (!shuttingDown) {
       retryConnect();
    }
}

void WormholeClient::onConnected()
{
    qDebug() << "WebSocket connected";
    retryCount = 0;

    QObject::connect(m_webSocket, &QWebSocket::textMessageReceived,
                        this, &WormholeClient::onTextMessageReceived);

    auto payload = QJsonDocument( QJsonObject {
        {"register", code}
    }).toJson();

    m_webSocket->sendTextMessage(payload);

    // On connected, we'll also create a timer to ping it every 4 minutes, since the websocket 
    // will timeout after 5 minutes
    timer = new QTimer(parent);
    QObject::connect(timer, &QTimer::timeout, [=]() {
        if (!shuttingDown && m_webSocket->isValid()) {
            auto payload = QJsonDocument(QJsonObject {
                {"ping", "ping"}
            }).toJson();
            m_webSocket->sendTextMessage(payload);
        }
    });
    timer->start(4 * 60 * 1000); // 4 minutes
}

void WormholeClient::onTextMessageReceived(QString message)
{
    AppDataServer::getInstance()->processMessage(message, parent, std::make_shared<ClientWebSocket>(m_webSocket), AppConnectionType::INTERNET);
}


// ==============================
// AppDataServer
// ==============================
AppDataServer* AppDataServer::instance = nullptr; 

QString AppDataServer::getWormholeCode(QString secretHex) {
    unsigned char* secret = new unsigned char[crypto_secretbox_KEYBYTES];
    sodium_hex2bin(secret, crypto_secretbox_KEYBYTES, secretHex.toStdString().c_str(), crypto_secretbox_KEYBYTES*2, 
        NULL, NULL, NULL);

    unsigned char* out1 = new unsigned char[crypto_hash_sha256_BYTES];
    crypto_hash_sha256(out1, secret, crypto_secretbox_KEYBYTES);

    unsigned char* out2 = new unsigned char[crypto_hash_sha256_BYTES];
    crypto_hash_sha256(out2, out1, crypto_hash_sha256_BYTES);

    char* wmcode = new char[crypto_hash_sha256_BYTES*2 + 1];
    sodium_bin2hex(wmcode, crypto_hash_sha256_BYTES*2 + 1, out2, crypto_hash_sha256_BYTES);    

    QString wmcodehex(wmcode);

    delete[] wmcode;
    delete[] out2;
    delete[] out1;
    delete[] secret;

    return wmcodehex;
}

QString AppDataServer::getSecretHex() {
    QSettings s;

    return s.value("mobileapp/secret", "").toString();
}

void AppDataServer::saveNewSecret(QString secretHex) {
    QSettings().setValue("mobileapp/secret", secretHex);

    if (secretHex.isEmpty())
        setAllowInternetConnection(false);
}

bool AppDataServer::getAllowInternetConnection() {
    return QSettings().value("mobileapp/allowinternet", false).toBool();
}

void AppDataServer::setAllowInternetConnection(bool allow) {
    QSettings().setValue("mobileapp/allowinternet", allow);
}

void AppDataServer::saveLastConnectedOver(AppConnectionType type) {
    QSettings().setValue("mobileapp/lastconnectedover", type);
}

AppConnectionType AppDataServer::getLastConnectionType() {
    return (AppConnectionType) QSettings().value("mobileapp/lastconnectedover", AppConnectionType::DIRECT).toInt();
}

void AppDataServer::saveLastSeenTime() {
    QSettings().setValue("mobileapp/lastseentime", QDateTime::currentSecsSinceEpoch());
}

QDateTime  AppDataServer::getLastSeenTime() {
    return QDateTime::fromSecsSinceEpoch(QSettings().value("mobileapp/lastseentime", 0).toLongLong());
}

void AppDataServer::setConnectedName(QString name) {
    QSettings().setValue("mobileapp/connectedname", name);
}

QString AppDataServer::getConnectedName() {
    return QSettings().value("mobileapp/connectedname", "").toString();
}

bool AppDataServer::isAppConnected() {
    return !getConnectedName().isEmpty() && 
        getLastSeenTime().daysTo(QDateTime::currentDateTime()) < 14;
}

void AppDataServer::connectAppDialog(MainWindow* parent) {
    QDialog d(parent);
    ui = new Ui_MobileAppConnector();
    ui->setupUi(&d);
    Settings::saveRestore(&d);

    updateUIWithNewQRCode(parent);
    updateConnectedUI();

    QObject::connect(ui->btnDisconnect, &QPushButton::clicked, [=] () {
        QSettings().setValue("mobileapp/connectedname", "");
        saveNewSecret("");

        updateConnectedUI();
    });
    
    QObject::connect(ui->txtConnStr, &QLineEdit::cursorPositionChanged, [=](int, int) {
        ui->txtConnStr->selectAll();
    });

    QObject::connect(ui->chkInternetConn, &QCheckBox::stateChanged, [=] (int state) {
        if (state == Qt::Checked) {

        }
        updateUIWithNewQRCode(parent);
    });

    // If we're not listening for the app, then start the websockets
    if (!parent->isWebsocketListening()) {
        QString wormholecode = "";
        if (getAllowInternetConnection())
            wormholecode = AppDataServer::getInstance()->getWormholeCode(AppDataServer::getInstance()->getSecretHex());

        parent->createWebsocket(wormholecode);
    }

    d.exec();

    // If there is nothing connected when the dialog exits, then shutdown the websockets
    if (!isAppConnected()) {
        parent->stopWebsocket();
    }

    // Cleanup
    tempSecret = "";
    
    delete tempWormholeClient;
    tempWormholeClient = nullptr;

    delete ui;
    ui = nullptr;
}

void AppDataServer::updateUIWithNewQRCode(MainWindow* mainwindow) {
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
    QString codeStr = uri + "," + secretStr;

    if (ui->chkInternetConn->isChecked()) {
        codeStr = codeStr + ",1";
    }

    registerNewTempSecret(secretStr, ui->chkInternetConn->isChecked(), mainwindow);

    ui->qrcode->setQrcodeString(codeStr);
    ui->txtConnStr->setText(codeStr);
}

void AppDataServer::registerNewTempSecret(QString tmpSecretHex, bool allowInternet, MainWindow* main) {
    tempSecret = tmpSecretHex;

    delete tempWormholeClient;
    tempWormholeClient = nullptr;

    if (allowInternet)
        tempWormholeClient = new WormholeClient(main, getWormholeCode(tempSecret));
}

QString AppDataServer::connDesc(AppConnectionType t) {
    if (t == AppConnectionType::DIRECT) {
        return QObject::tr("Connected directly");
    }
    else {
        return QObject::tr("Connected over the internet via ZecWallet wormhole service");
    }
}

void AppDataServer::updateConnectedUI() {
    if (ui == nullptr)
        return;

    auto remoteName = getConnectedName();

    ui->lblRemoteName->setText(remoteName.isEmpty() ?  "(Not connected to any device)" : remoteName);
    ui->lblLastSeen->setText(remoteName.isEmpty() ? "" : getLastSeenTime().toString(Qt::SystemLocaleLongDate));
    ui->lblConnectionType->setText(remoteName.isEmpty() ? "" : connDesc(getLastConnectionType()));

    ui->btnDisconnect->setEnabled(!remoteName.isEmpty());
}

QString AppDataServer::getNonceHex(NonceType nt) {
    QSettings s;
    QString hex;
    if (nt == NonceType::LOCAL) {
        // The default local nonce starts from 1, to always keep it odd
        auto defaultLocalNonce = "01" + QString("00").repeated(crypto_secretbox_NONCEBYTES-1);
        hex = s.value("mobileapp/localnoncehex", defaultLocalNonce).toString();
    }
    else {
        hex = s.value("mobileapp/remotenoncehex", QString("00").repeated(crypto_secretbox_NONCEBYTES)).toString();
    }
    return hex;
}

void AppDataServer::saveNonceHex(NonceType nt, QString noncehex) {
    QSettings s;
    assert(noncehex.length() == crypto_secretbox_NONCEBYTES * 2);
    if (nt == NonceType::LOCAL) {
        s.setValue("mobileapp/localnoncehex", noncehex);
    }
    else {
        s.setValue("mobileapp/remotenoncehex", noncehex);
    }
    s.sync();
}

// Encrypt an outgoing message with the stored secret key.
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

    auto json =  QJsonDocument(QJsonObject{
            {"nonce", QString(newLocalNonce)},
            {"payload", QString(encryptedHex)},
            {"to", getWormholeCode(getSecretHex())}
        });
    
    delete[] noncebin;
    delete[] newLocalNonce;
    delete[] secret;
    delete[] encrpyted;
    delete[] encryptedHex;

    return json.toJson();
}

/**
  Attempt to decrypt a message. If the decryption fails, it returns the string "error", the decrypted message otherwise. 
  It will use the given secret to attempt decryption. In addition, it will enforce that the nonce is greater than the last seen nonce, 
  unless the skipNonceCheck = true, which is used when attempting decrtption with a temp secret key.
*/
QString AppDataServer::decryptMessage(QJsonDocument msg, QString secretHex, QString lastRemoteNonceHex) {
    // Decrypt and then process
    QString noncehex = msg.object().value("nonce").toString();
    QString encryptedhex = msg.object().value("payload").toString();

    // Enforce limits on the size of the message
    if (noncehex.length() > ((int)crypto_secretbox_NONCEBYTES * 2) ||
        encryptedhex.length() > 2 * 50 * 1024 /*50kb*/) {
        return "error";
    }

    // Check to make sure that the nonce is greater than the last known remote nonce
    unsigned char* lastRemoteBin = new unsigned char[crypto_secretbox_NONCEBYTES];
    sodium_hex2bin(lastRemoteBin, crypto_secretbox_NONCEBYTES, lastRemoteNonceHex.toStdString().c_str(), lastRemoteNonceHex.length(),
        NULL, NULL, NULL);

    unsigned char* noncebin = new unsigned char[crypto_secretbox_NONCEBYTES];
    sodium_hex2bin(noncebin, crypto_secretbox_NONCEBYTES, noncehex.toStdString().c_str(), noncehex.length(),
        NULL, NULL, NULL);

    assert(crypto_secretbox_KEYBYTES == crypto_hash_sha256_BYTES);
    if (sodium_compare(lastRemoteBin, noncebin, crypto_secretbox_NONCEBYTES) != -1) {
        // Refuse to accept a lower nonce, return an error
        delete[] lastRemoteBin;
        delete[] noncebin;
        return "error";
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
        saveLastSeenTime();

        char* decryptedStr = new char[decryptedLen + 1];
        sodium_memzero(decryptedStr, decryptedLen + 1);
        memcpy(decryptedStr, decrypted, decryptedLen);

        payload = QString(decryptedStr);

        delete[] decryptedStr;
    }

    delete[] secret;
    delete[] lastRemoteBin;
    delete[] noncebin;
    delete[] encrypted;
    delete[] decrypted;
    
    return payload;
}

// Process an incoming text message. The message has to be encrypted with the secret key (or the temporary secret key)
void AppDataServer::processMessage(QString message, MainWindow* mainWindow, std::shared_ptr<ClientWebSocket> pClient, AppConnectionType connType) {
    auto replyWithError = [=]() {
        auto r = QJsonDocument(QJsonObject{
                    {"error", "Encryption error"},
                    {"to", getWormholeCode(getSecretHex())}
            }).toJson();
            pClient->sendTextMessage(r);
            return;
    };
    
    // First, extract the command from the message
    auto msg = QJsonDocument::fromJson(message.toUtf8());

    // Check if we got an error from the websocket
    if (msg.object().contains("error")) {
        qDebug() << "Error:" << msg.toJson();
        return;
    }

    // If the message is a ping, just ignore it
    if (msg.object().contains("ping")) {
        return;
    }

    // Then, check if the message is encrpted
    if (!msg.object().contains("nonce")) {
        replyWithError();
        return;
    }

    auto decrypted = decryptMessage(msg, getSecretHex(), getNonceHex(NonceType::REMOTE));

    // If the decryption failed, maybe this is a new connection, so see if the dialog is open and a 
    // temp secret is in place
    if (decrypted == "error") {
        // If the dialog is open, then there might be a temporary, new secret key. Attempt to decrypt
        // with that.
        if (!tempSecret.isEmpty()) {
            // Since this is a temp secret, the last seen nonce will be "0", so basically we'll accept any nonce
            QString zeroNonce = QString("00").repeated(crypto_secretbox_NONCEBYTES);
            decrypted = decryptMessage(msg, tempSecret, zeroNonce);
            if (decrypted == "error") {
                // Oh, well. Just return an error
                replyWithError();
                return;
            }
            else {
                // This is a new connection. So, update the the secret. Note the last seen remote nonce has already been updated by
                // decryptMessage()
                saveNewSecret(tempSecret);
                setAllowInternetConnection(tempWormholeClient != nullptr);

                // Swap out the wormhole connection
                mainWindow->replaceWormholeClient(tempWormholeClient);
                tempWormholeClient = nullptr;

                saveLastConnectedOver(connType);
                processDecryptedMessage(decrypted, mainWindow, pClient);

                // If the Connection UI is showing, we have to update the UI as well
                if (ui != nullptr) {
                    // Update the connected phone information
                    updateConnectedUI();

                    // Update with a new QR Code for safety, so this secret isn't used by anyone else
                    updateUIWithNewQRCode(mainWindow);
                }

                return;
            }
        }
        else {
            replyWithError();
            return;
        }
    } else {
        saveLastConnectedOver(connType);
        processDecryptedMessage(decrypted, mainWindow, pClient);
        return;
    }
}

// Decrypted method will be executed here. 
void AppDataServer::processDecryptedMessage(QString message, MainWindow* mainWindow, std::shared_ptr<ClientWebSocket> pClient) {
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

// "sendTx" command. This method will actually send money, so be careful with everything
void AppDataServer::processSendTx(QJsonObject sendTx, MainWindow* mainwindow, std::shared_ptr<ClientWebSocket> pClient) {
    auto error = [=](QString reason) {
        auto r = QJsonDocument(QJsonObject{
           {"errorCode", -1},
           {"errorMessage", "Couldn't send Tx:" + reason}
        }).toJson();
        pClient->sendTextMessage(encryptOutgoing(r));
        return;
    };

    // Refuse to send if the node is still syncing
    if (Settings::getInstance()->isSyncing()) {
        error(QObject::tr("Node is still syncing."));
        return;
    }

    // Create a Tx Object
    Tx tx;
    tx.fee = Settings::getMinerFee();

    // Find a from address that has at least the sending amout
    double amt = sendTx["amount"].toString().toDouble();
    auto allBalances = mainwindow->getRPC()->getModel()->getAllBalances();
    QList<QPair<QString, double>> bals;
    for (auto i : allBalances.keys()) {
        // Filter out sprout addresses
        if (Settings::getInstance()->isSproutAddress(i))
            continue;
        // Filter out balances that don't have the requisite amount
        if (allBalances.value(i) < amt)
            continue;

        bals.append(QPair<QString, double>(i, allBalances.value(i)));
    }

    if (bals.isEmpty()) {
        error(QObject::tr("No sapling or transparent addresses with enough balance to spend."));
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
        [=] (QString) {}, 
        // Submitted Tx successfully
        [=] (QString, QString txid) {
            auto r = QJsonDocument(QJsonObject{
               {"version", 1.0},
               {"command", "sendTxSubmitted"},
               {"txid",  txid}
            }).toJson();
            pClient->sendTextMessage(encryptOutgoing(r));
        },
        // Errored while submitting Tx
        [=] (QString, QString errStr) {
            auto r = QJsonDocument(QJsonObject{
               {"version", 1.0},
               {"command", "sendTxFailed"},
               {"err",  errStr}
            }).toJson();
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

// "getInfo" command
void AppDataServer::processGetInfo(QJsonObject jobj, MainWindow* mainWindow, std::shared_ptr<ClientWebSocket> pClient) {
    auto connectedName = jobj["name"].toString();
    
    if (mainWindow == nullptr || mainWindow->getRPC() == nullptr) {
        pClient->close(QWebSocketProtocol::CloseCodeNormal, "Not yet ready");
        return;
    }

    // Max spendable safely from a z address and from any address
    double maxZSpendable = 0;
    double maxSpendable = 0;
    for (auto a : mainWindow->getRPC()->getModel()->getAllBalances().keys()) {
        if (Settings::getInstance()->isSaplingAddress(a)) {
            if (mainWindow->getRPC()->getModel()->getAllBalances().value(a) > maxZSpendable) {
                maxZSpendable = mainWindow->getRPC()->getModel()->getAllBalances().value(a);
            }
        }
        if (mainWindow->getRPC()->getModel()->getAllBalances().value(a) > maxSpendable) {
            maxSpendable = mainWindow->getRPC()->getModel()->getAllBalances().value(a);
        }
    }

    setConnectedName(connectedName);

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

void AppDataServer::processGetTransactions(MainWindow* mainWindow, std::shared_ptr<ClientWebSocket> pClient) {
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
AppDataModel* AppDataModel::instance = nullptr;
