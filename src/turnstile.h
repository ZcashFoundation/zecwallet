#ifndef TURNSTILE_H
#define TURNSTILE_H

#include "precompiled.h"

class MainWindow;

class Turnstile
{
public:
    static void        showZcashdMigration(MainWindow* parent);

    static double       minMigrationAmount;   
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
