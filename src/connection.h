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
    bool    usingZcashConf;

    ConnectionType connType;
};

class Connection;

class ConnectionLoader {

public:
    ConnectionLoader(MainWindow* main, RPC* rpc);
    ~ConnectionLoader();

    void loadConnection();

private:
    std::shared_ptr<ConnectionConfig> autoDetectZcashConf();
    std::shared_ptr<ConnectionConfig> loadFromSettings();

    Connection* makeConnection(std::shared_ptr<ConnectionConfig> config);

    void refreshZcashdState(Connection* connection);
    int  getProgressFromStatus(QString status);

    void showError(QString explanation);

    QDialog*                d;
    Ui_ConnectionDialog*    connD;

    MainWindow*             main;
    RPC*                    rpc;
};

/**
 * Represents a connection to a zcashd. It may even start a new zcashd if needed.
 * This is also a UI class, so it may show a dialog waiting for the connection.
*/
class Connection {
public:
    Connection(QNetworkAccessManager* c, QNetworkRequest* r, std::shared_ptr<ConnectionConfig> conf);
    ~Connection();


    QNetworkAccessManager*              restclient;
    QNetworkRequest*                    request;
    std::shared_ptr<ConnectionConfig>   config;

    void doRPC(const json& payload, const std::function<void(json)>& cb, 
               const std::function<void(QNetworkReply::NetworkError, const json&)>& ne);

};

#endif