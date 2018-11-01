#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include "precompiled.h"

#include "unspentoutput.h"
#include "balancestablemodel.h"
#include "txtablemodel.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "connection.h"

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
    RPC(MainWindow* main);
    ~RPC();

    void setConnection(Connection* c);

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

    void newZaddr(bool sapling, const std::function<void(json)>& cb);
    void newTaddr(const std::function<void(json)>& cb);

    void getZPrivKey(QString addr, const std::function<void(json)>& cb);
    void getTPrivKey(QString addr, const std::function<void(json)>& cb);
    void importZPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb);
    void importTPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb);

    Turnstile*  getTurnstile()  { return turnstile; }
    Connection* getConnection() { return conn; }

private:
    void noConnection();

    void refreshBalances();

    void refreshTransactions();    
    void refreshSentZTrans();
    void refreshReceivedZTrans(QList<QString> zaddresses);

    bool processUnspent     (const json& reply);
    void updateUI           (bool anyUnconfirmed);

    void getInfoThenRefresh(bool force);

    void getBalance(const std::function<void(json)>& cb);

    void getTransparentUnspent  (const std::function<void(json)>& cb);
    void getZUnspent            (const std::function<void(json)>& cb);
    void getTransactions        (const std::function<void(json)>& cb);
    void getZAddresses          (const std::function<void(json)>& cb);

    void handleConnectionError  (const QString& error);
    void handleTxError          (const QString& error);

    Connection*                 conn                        = nullptr;

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
