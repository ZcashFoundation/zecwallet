#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include "precompiled.h"

#include "datamodel.h"
#include "balancestablemodel.h"
#include "txtablemodel.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "connection.h"

using json = nlohmann::json;

class Turnstile;

struct TransactionItem {
    QString         type;
    qint64          datetime;
    QString         address;
    QString         txid;
    double          amount;
    long            confirmations;
    QString         fromAddr;
    QString         memo;
};

struct WatchedTx {
    QString opid;
    Tx tx;
    std::function<void(QString, QString)> completed;
    std::function<void(QString, QString)> error;
};

struct MigrationStatus {
    bool            available;     // Whether the underlying zcashd supports migration?
    bool            enabled;
    QString         saplingAddress;
    double          unmigrated;
    double          migrated;
    QList<QString>  txids;
};

class RPC
{
public:
    RPC(MainWindow* main);
    ~RPC();

    DataModel* getModel() { return model; }

    void setConnection(Connection* c);
    void setEZcashd(QProcess* p);
    const QProcess* getEZcashD() { return ezcashd; }

    void refresh(bool force = false);

    void refreshAddresses();    


    
    void checkForUpdate(bool silent = true);
    void refreshZECPrice();
    void getZboardTopics(std::function<void(QMap<QString, QString>)> cb);

    void executeStandardUITransaction(Tx tx); 

    void executeTransaction(Tx tx, 
        const std::function<void(QString opid)> submitted,
        const std::function<void(QString opid, QString txid)> computed,
        const std::function<void(QString opid, QString errStr)> error);

    void fillTxJsonParams(json& params, Tx tx);
    void sendZTransaction(json params, const std::function<void(json)>& cb, const std::function<void(QString)>& err);
    void watchTxStatus();

    const QMap<QString, WatchedTx> getWatchingTxns() { return watchingOps; }
    void addNewTxToWatch(const QString& newOpid, WatchedTx wtx); 

    const TxTableModel*               getTransactionsModel() { return transactionsTableModel; }

    void newZaddr(bool sapling, const std::function<void(json)>& cb);
    void newTaddr(const std::function<void(json)>& cb);

    void getZPrivKey(QString addr, const std::function<void(json)>& cb);
    void getTPrivKey(QString addr, const std::function<void(json)>& cb);
    void importZPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb);
    void importTPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb);
    void validateAddress(QString address, const std::function<void(json)>& cb);

    void shutdownZcashd();
    void noConnection();
    bool isEmbedded() { return ezcashd != nullptr; }

    QString getDefaultSaplingAddress();
    QString getDefaultTAddress();

    void getAllPrivKeys(const std::function<void(QList<QPair<QString, QString>>)>);

    Turnstile*  getTurnstile()  { return turnstile; }
    Connection* getConnection() { return conn; }

    const MigrationStatus*      getMigrationStatus() { return &migrationStatus; }
    void                        setMigrationStatus(bool enabled);

private:
    void refreshBalances();

    void refreshTransactions();    
    void refreshMigration();
    void refreshSentZTrans();
    void refreshReceivedZTrans(QList<QString> zaddresses);

    bool processUnspent     (const json& reply, QMap<QString, double>* newBalances, QList<UnspentOutput>* newUtxos);
    void updateUI           (bool anyUnconfirmed);

    void getInfoThenRefresh(bool force);

    void getBalance(const std::function<void(json)>& cb);

    void getTransparentUnspent  (const std::function<void(json)>& cb);
    void getZUnspent            (const std::function<void(json)>& cb);
    void getTransactions        (const std::function<void(json)>& cb);
    void getZAddresses          (const std::function<void(json)>& cb);
    void getTAddresses          (const std::function<void(json)>& cb);

    Connection*                 conn                        = nullptr;
    QProcess*                   ezcashd                     = nullptr;

    QMap<QString, WatchedTx>    watchingOps;

    TxTableModel*               transactionsTableModel      = nullptr;
    BalancesTableModel*         balancesTableModel          = nullptr;

    DataModel*                  model;

    QTimer*                     timer;
    QTimer*                     txTimer;
    QTimer*                     priceTimer;

    Ui::MainWindow*             ui;
    MainWindow*                 main;
    Turnstile*                  turnstile;

    // Sapling turnstile migration status (for the zcashd v2.0.5 tool)
    MigrationStatus             migrationStatus;

    // Current balance in the UI. If this number updates, then refresh the UI
    QString                     currentBalance;
};

#endif // RPCCLIENT_H
