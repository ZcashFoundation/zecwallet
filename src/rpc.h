#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include "precompiled.h"

#include "unspentoutput.h"
#include "balancestablemodel.h"
#include "txtablemodel.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"

using json = nlohmann::json;


struct TransactionItem {
    QString         type;
    unsigned long   datetime;
    QString         address;
    QString         txid;
    double          amount;
    unsigned long   confirmations;
};

class RPC
{
public:
    RPC(QNetworkAccessManager* restclient, MainWindow* main);    
    ~RPC();

    void refresh();             // Refresh all transactions
    void refreshAddresses();    // Refresh wallet Z-addrs
    void refreshZECPrice();

    void sendZTransaction   (json params, const std::function<void(json)>& cb);
    void watchTxStatus();
    void addNewTxToWatch(Tx tx, const QString& newOpid); 

    BalancesTableModel*               getBalancesModel()  { return balancesTableModel; }    
    const QList<QString>*             getAllZAddresses()  { return zaddresses; }
    const QList<UnspentOutput>*       getUTXOs()          { return utxos; }
    const QMap<QString, double>*      getAllBalances()    { return allBalances; }

	void reloadConnectionInfo();

    void newZaddr               (bool sapling, const std::function<void(json)>& cb);
    void newTaddr               (const std::function<void(json)>& cb);
    
private:
    void doRPC      (const json& payload, const std::function<void(json)>& cb);
    void doSendRPC  (const json& payload, const std::function<void(json)>& cb);

    void refreshBalances();

    void refreshTransactions();    
    void refreshSentZTrans      (QList<QString> zaddresses);
    void refreshReceivedZTrans  (QList<QString> zaddrs, QList<QString> txidFilter);

	bool processUnspent	(const json& reply);
	void updateUI		(bool anyUnconfirmed);

    void getInfoThenRefresh();

    void getBalance(const std::function<void(json)>& cb);

    void getTransparentUnspent  (const std::function<void(json)>& cb);
    void getZUnspent            (const std::function<void(json)>& cb);
    void getTransactions        (const std::function<void(json)>& cb);
    void getZAddresses          (const std::function<void(json)>& cb);

    void handleConnectionError  (const QString& error);
    void handleTxError          (const QString& error);

    // Batch
    void                getBatchRPC(const QList<QString>& payloads,
                                    std::function<json(QString)> payloadGenerator,
                                    std::function<void(QMap<QString, json>*)> cb);

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

    // Current balance in the UI. If this number updates, then refresh the UI
    QString                     currentBalance;
    // First time warning flag for no connection
    bool                        firstTime = true;
};

#endif // RPCCLIENT_H
