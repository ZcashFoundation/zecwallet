#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include "precompiled.h"

#include "unspentoutput.h"
#include "balancestablemodel.h"
#include "txtablemodel.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"

using json = nlohmann::json;

class Turnstile;

struct TransactionItem {
    QString         type;
    qint64            datetime;
    QString         address;
    QString         txid;
    double          amount;
    unsigned long   confirmations;
    QString         fromAddr;
    QString         memo;
};

class RPC
{
public:
    RPC(QNetworkAccessManager* restclient, MainWindow* main);    
    ~RPC();

    void refresh(bool force = false);

    void refreshAddresses();    
    void refreshZECPrice();

    void fillTxJsonParams(json& params, Tx tx);
    void sendZTransaction   (json params, const std::function<void(json)>& cb);
    void watchTxStatus();
    void addNewTxToWatch(Tx tx, const QString& newOpid); 

    BalancesTableModel*               getBalancesModel()  { return balancesTableModel; }    
    const QList<QString>*             getAllZAddresses()  { return zaddresses; }
    const QList<UnspentOutput>*       getUTXOs()          { return utxos; }
    const QMap<QString, double>*      getAllBalances()    { return allBalances; }

    void reloadConnectionInfo();

    void newZaddr(bool sapling, const std::function<void(json)>& cb);
    void newTaddr(const std::function<void(json)>& cb);


    void getZPrivKey(QString addr, const std::function<void(json)>& cb);
    void getTPrivKey(QString addr, const std::function<void(json)>& cb);
    void importZPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb);
    void importTPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb);

    Turnstile* getTurnstile() { return turnstile; }
    
    // Batch method. Note: Because of the template, it has to be in the header file. 
    template<class T>
    void getBatchRPC(const QList<T>& payloads,
                     std::function<json(T)> payloadGenerator,
                     std::function<void(QMap<T, json>*)> cb) {    
        auto responses = new QMap<T, json>(); // zAddr -> list of responses for each call. 
        int totalSize = payloads.size();

        for (auto item: payloads) {
            json payload = payloadGenerator(item);
            
            QNetworkReply *reply = restclient->post(request, QByteArray::fromStdString(payload.dump()));

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



private:
    void doRPC      (const json& payload, const std::function<void(json)>& cb);
    void doSendRPC  (const json& payload, const std::function<void(json)>& cb);
    void doSendRPC(const json& payload, const std::function<void(json)>& cb, const std::function<void(QString)>& err);

    void refreshBalances();

    void refreshTransactions();    
    void refreshSentZTrans      ();
    void refreshReceivedZTrans  (QList<QString> zaddresses);

    bool processUnspent    (const json& reply);
    void updateUI        (bool anyUnconfirmed);

    void getInfoThenRefresh(bool force);

    void getBalance(const std::function<void(json)>& cb);

    void getTransparentUnspent  (const std::function<void(json)>& cb);
    void getZUnspent            (const std::function<void(json)>& cb);
    void getTransactions        (const std::function<void(json)>& cb);
    void getZAddresses          (const std::function<void(json)>& cb);

    void handleConnectionError  (const QString& error);
    void handleTxError          (const QString& error);

    QNetworkAccessManager*      restclient;    
    QNetworkRequest             request;

    QList<UnspentOutput>*       utxos                       = nullptr;
    QMap<QString, double>*      allBalances                 = nullptr;
    QList<QString>*             zaddresses                  = nullptr;
    
    QMap<QString, Tx>           watchingOps;

    TxTableModel*               transactionsTableModel      = nullptr;
    BalancesTableModel*         balancesTableModel          = nullptr;

    QTimer*                     timer;
    QTimer*                     txTimer;
    QTimer*                     priceTimer;

    Ui::MainWindow*             ui;
    MainWindow*                 main;
    Turnstile*                  turnstile;

    // Current balance in the UI. If this number updates, then refresh the UI
    QString                     currentBalance;
    // First time warning flag for no connection
    bool                        firstTime = true;
};

#endif // RPCCLIENT_H
