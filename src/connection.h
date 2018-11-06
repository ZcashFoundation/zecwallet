#ifndef CONNECTION_H
#define CONNECTION_H

#include "mainwindow.h"
#include "ui_connection.h"
#include "precompiled.h"

using json = nlohmann::json;

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

    void doAutoConnect();
    void doManualConnect();

    void createZcashConf();
    QString locateZcashConfFile();
    QString zcashConfWritableLocation();
    QString zcashParamsDir();

    void downloadParams(std::function<void(void)> cb);
    void doNextDownload(std::function<void(void)> cb);
    bool startEmbeddedZcashd();

    void refreshZcashdState(Connection* connection);
    int  getProgressFromStatus(QString status);

    void showError(QString explanation);
    void showInformation(QString info, QString detail = "");

    void doRPCSetConnection(Connection* conn);

    QProcess*               ezcashd  = nullptr;

    QDialog*                d;
    Ui_ConnectionDialog*    connD;

    MainWindow*             main;
    RPC*                    rpc;

    QNetworkReply* currentDownload = nullptr;
    QFile*         currentOutput   = nullptr;
    QQueue<QUrl>*  downloadQueue   = nullptr;

    QNetworkAccessManager* client  = nullptr; 
    QTime downloadTime;
};

/**
 * Represents a connection to a zcashd. It may even start a new zcashd if needed.
 * This is also a UI class, so it may show a dialog waiting for the connection.
*/
class Connection {
public:
    Connection(MainWindow* m, QNetworkAccessManager* c, QNetworkRequest* r, std::shared_ptr<ConnectionConfig> conf);
    ~Connection();

    QNetworkAccessManager*              restclient;
    QNetworkRequest*                    request;
    std::shared_ptr<ConnectionConfig>   config;
    MainWindow*                         main;

    void doRPC(const json& payload, const std::function<void(json)>& cb, 
               const std::function<void(QNetworkReply*, const json&)>& ne);
    void doRPCWithDefaultErrorHandling(const json& payload, const std::function<void(json)>& cb);
    void doRPCIgnoreError(const json& payload, const std::function<void(json)>& cb) ;

    void showTxError(const QString& error);

    // Batch method. Note: Because of the template, it has to be in the header file. 
    template<class T>
    void doBatchRPC(const QList<T>& payloads,
                     std::function<json(T)> payloadGenerator,
                     std::function<void(QMap<T, json>*)> cb) {    
        auto responses = new QMap<T, json>(); // zAddr -> list of responses for each call. 
        int totalSize = payloads.size();

        for (auto item: payloads) {
            json payload = payloadGenerator(item);
            
            QNetworkReply *reply = restclient->post(*request, QByteArray::fromStdString(payload.dump()));

            QObject::connect(reply, &QNetworkReply::finished, [=] {
                reply->deleteLater();
                
                auto all = reply->readAll();            
                auto parsed = json::parse(all.toStdString(), nullptr, false);

                if (reply->error() != QNetworkReply::NoError) {            
                    qDebug() << QString::fromStdString(parsed.dump());
                    qDebug() << reply->errorString();

                    (*responses)[item] = json::object();    // Empty object
                } else {
                    if (parsed.is_discarded()) {
                        (*responses)[item] = json::object();    // Empty object
                    } else {
                        (*responses)[item] = parsed["result"];
                    }
                }
            });
        }

        auto waitTimer = new QTimer(main);
        QObject::connect(waitTimer, &QTimer::timeout, [=]() {
            if (responses->size() == totalSize) {
                waitTimer->stop();

                cb(responses);

                waitTimer->deleteLater();            
            }
        });
        waitTimer->start(100);    
    }
};

#endif
