#ifndef BALANCESTABLEMODEL_H
#define BALANCESTABLEMODEL_H

#include "precompiled.h"

struct UnspentOutput {
    QString address;
    QString txid;
    QString amount;    
    int     confirmations;
    bool    spendable;
};

class BalancesTableModel : public QAbstractTableModel
{
public:
    BalancesTableModel(QObject* parent);
    ~BalancesTableModel();

    void setNewData(const QMap<QString, double>* balances, const QList<UnspentOutput>* outputs);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QList<std::tuple<QString, double>>*    modeldata   = nullptr;    
    QList<UnspentOutput>*                  utxos       = nullptr;  

    bool loading = true;
};

#endif // BALANCESTABLEMODEL_H