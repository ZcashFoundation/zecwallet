#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include "precompiled.h"

#include "datamodel.h"
#include "balancestablemodel.h"
#include "txtablemodel.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "zcashdrpc.h"
#include "connection.h"

using json = nlohmann::json;

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

class Controller
{
public:
    Controller(MainWindow* main);
    ~Controller();

    DataModel* getModel() { return model; }

    Connection* getConnection() { return zrpc->getConnection(); }
    void setConnection(Connection* c);

    void setEZcashd(QProcess* p);
    const QProcess* getEZcashD() { return ezcashd; }

    void refresh(bool force = false);
    void refreshAddresses();    
    
    void checkForUpdate(bool silent = true);
    void refreshZECPrice();
    //void getZboardTopics(std::function<void(QMap<QString, QString>)> cb);

    void executeStandardUITransaction(Tx tx); 

    void executeTransaction(Tx tx, 
        const std::function<void(QString opid)> submitted,
        const std::function<void(QString opid, QString txid)> computed,
        const std::function<void(QString opid, QString errStr)> error);

    void fillTxJsonParams(json& params, Tx tx);
    
    void watchTxStatus();

    const QMap<QString, WatchedTx> getWatchingTxns() { return watchingOps; }
    void addNewTxToWatch(const QString& newOpid, WatchedTx wtx); 

    const TxTableModel*               getTransactionsModel() { return transactionsTableModel; }

    void shutdownZcashd();
    void noConnection();
    bool isEmbedded() { return ezcashd != nullptr; }

    void createNewZaddr(bool sapling, const std::function<void(json)>& cb) { zrpc->createNewZaddr(sapling, cb); }
    void createNewTaddr(const std::function<void(json)>& cb) { zrpc->createNewTaddr(cb); }

    void validateAddress(QString address, const std::function<void(json)>& cb) { zrpc->validateAddress(address, cb); }

    void fetchZPrivKey(QString addr, const std::function<void(json)>& cb) { zrpc->fetchZPrivKey(addr, cb); }
    void fetchTPrivKey(QString addr, const std::function<void(json)>& cb) { zrpc->fetchTPrivKey(addr, cb); }
    void fetchAllPrivKeys(const std::function<void(QList<QPair<QString, QString>>)> cb) { zrpc->fetchAllPrivKeys(cb); }

    void importZPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb) { zrpc->importZPrivKey(addr, rescan, cb); }
    void importTPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb) { zrpc->importTPrivKey(addr, rescan, cb); }

    QString getDefaultSaplingAddress();
    QString getDefaultTAddress();   
    
    const MigrationStatus*      getMigrationStatus() { return &migrationStatus; }
    void  setMigrationStatus(bool status) { zrpc->setMigrationStatus(status); }
    
private:
    void refreshBalances();

    void refreshTransactions();    
    void refreshMigration();
    void refreshSentZTrans();
    void refreshReceivedZTrans(QList<QString> zaddresses);

    bool processUnspent     (const json& reply, QMap<QString, double>* newBalances, QList<UnspentOutput>* newUtxos);
    void updateUI           (bool anyUnconfirmed);

    void getInfoThenRefresh(bool force);
    
    QProcess*                   ezcashd                     = nullptr;

    QMap<QString, WatchedTx>    watchingOps;

    TxTableModel*               transactionsTableModel      = nullptr;
    BalancesTableModel*         balancesTableModel          = nullptr;

    DataModel*                  model;
    ZcashdRPC*                  zrpc;

    QTimer*                     timer;
    QTimer*                     txTimer;
    QTimer*                     priceTimer;

    Ui::MainWindow*             ui;
    MainWindow*                 main;

    // Sapling turnstile migration status (for the zcashd v2.0.5 tool)
    MigrationStatus             migrationStatus;

    // Current balance in the UI. If this number updates, then refresh the UI
    QString                     currentBalance;
};

#endif // RPCCLIENT_H
