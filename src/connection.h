#ifndef CONNECTION_H
#define CONNECTION_H

#include "ui_connection.h"
#include "precompiled.h"

using json = nlohmann::json;

class MainWindow;
class RPC;

enum ConnectionType {
    DetectedConfExternalZcashD = 1,
    UISettingsZCashD,
    InternalZcashD
};

struct ConnectionConfig {
    QString host;
    QString port;
    QString rpcuser;
    QString rpcpassword;

    ConnectionType connType;
};

class LoadingDialog;

class ConnectionLoader {

public:
    ConnectionLoader(MainWindow* main);
    ~ConnectionLoader();

    void getConnection(std::function<void(RPC*)> cb);

private:
    std::shared_ptr<ConnectionConfig*> autoDetectZcashConf();
    std::shared_ptr<ConnectionConfig*> loadFromSettings();

    LoadingDialog*          d;
    Ui_ConnectionDialog*    connD;
    MainWindow*             main;
};

/**
 * Represents a connection to a zcashd. It may even start a new zcashd if needed.
 * This is also a UI class, so it may show a dialog waiting for the connection.
*/
class Connection {
public:
    Connection(QNetworkAccessManager* c, QNetworkRequest* r);
    ~Connection();


    QNetworkAccessManager*  restclient;
    QNetworkRequest*        request;

    void doRPC(const json& payload, const std::function<void(json)>& cb, 
               const std::function<void(QNetworkReply::NetworkError, const json&)>& ne);

};

#endif