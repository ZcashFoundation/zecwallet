#ifndef TURNSTILE_H
#define TURNSTILE_H

#include "precompiled.h"

class RPC;
class MainWindow;
struct Tx;


struct TurnstileMigrationItem {
    QString     fromAddr;
    QString        intTAddr;
    QString        destAddr;
    int            blockNumber;
    double        amount;
    int         status;
};

enum TurnstileMigrationItemStatus {
    NotStarted = 0,
    SentToT,
    SentToZS,
    NotEnoughBalance,
    UnknownError
};

struct ProgressReport {
    int     step;
    int     totalSteps;
    int     nextBlock;
    bool     hasErrors;
    QString from;
    QString to;
    QString via;
};

class Turnstile
{
public:
    Turnstile(RPC* _rpc, MainWindow* mainwindow);

    void               planMigration(QString zaddr, QString destAddr, int splits, int numBlocks);
    QList<double>      splitAmount(double amount, int parts);
    void               fillAmounts(QList<double>& amounts, double amount, int count);

    QList<TurnstileMigrationItem> readMigrationPlan();
    void               writeMigrationPlan(QList<TurnstileMigrationItem> plan);
    void               removeFile();
    
    void               executeMigrationStep();
    ProgressReport     getPlanProgress();
    bool               isMigrationPresent();

    static void        showZcashdMigration(MainWindow* parent);

    static double       minMigrationAmount;
private:
    QList<int>          getBlockNumbers(int start, int end, int count);
    QString             writeableFile();

    void                doSendTx(Tx tx, std::function<void(void)> cb);


    QList<TurnstileMigrationItem>::Iterator getNextStep(QList<TurnstileMigrationItem>& plan);    

    RPC*         rpc;    
    MainWindow* mainwindow;
};


// Classes for zcashd 2.0.5 native migration


class MigrationTxns : public QAbstractTableModel {

public:
    MigrationTxns(QTableView* parent, QList<QString> txids);
    ~MigrationTxns() = default;

    int      rowCount(const QModelIndex &parent) const;
    int      columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    QString getTxid(int row) const;

private:
    QList<QString> txids;
    QStringList headers;    
};


#endif
