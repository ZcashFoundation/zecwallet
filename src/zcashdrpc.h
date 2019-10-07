#ifndef ZCASHDRPC_H
#define ZCASHDRPC_H

#include "precompiled.h"

#include "connection.h"

using json = nlohmann::json;

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


class ZcashdRPC {
public:
    ZcashdRPC();
    ~ZcashdRPC();

    bool haveConnection();
    void setConnection(Connection* c);
    Connection* getConnection() { return conn; }

    void fetchTransparentUnspent  (const std::function<void(json)>& cb);
    void fetchZUnspent            (const std::function<void(json)>& cb);
    void fetchTransactions        (const std::function<void(json)>& cb);
    void fetchZAddresses          (const std::function<void(json)>& cb);
    void fetchTAddresses          (const std::function<void(json)>& cb);

    void fetchReceivedZTrans(QList<QString> zaddrs, const std::function<void(QString)> usedAddrFn,
        const std::function<void(QList<TransactionItem>)> txdataFn);
    void fetchReceivedTTrans(QList<QString> txids, QList<TransactionItem> sentZtxs,
    const std::function<void(QList<TransactionItem>)> txdataFn);

    void fetchInfo(const std::function<void(json)>& cb, 
                    const std::function<void(QNetworkReply*, const json&)>& err);
    void fetchBlockchainInfo(const std::function<void(json)>& cb);
    void fetchNetSolOps(const std::function<void(qint64)> cb);
    void fetchOpStatus(const std::function<void(json)>& cb);

    void fetchMigrationStatus(const std::function<void(json)>& cb);
    void setMigrationStatus(bool enabled);

    void fetchBalance(const std::function<void(json)>& cb);

    void createNewZaddr(bool sapling, const std::function<void(json)>& cb);
    void createNewTaddr(const std::function<void(json)>& cb);

    void fetchZPrivKey(QString addr, const std::function<void(json)>& cb);
    void fetchTPrivKey(QString addr, const std::function<void(json)>& cb);
    void importZPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb);
    void importTPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb);
    void validateAddress(QString address, const std::function<void(json)>& cb);

    void fetchAllPrivKeys(const std::function<void(QList<QPair<QString, QString>>)>);

    void sendZTransaction(json params, const std::function<void(json)>& cb, const std::function<void(QString)>& err);

private:
    Connection*  conn                        = nullptr;
};

#endif // ZCASHDRPC_H
