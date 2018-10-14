#ifndef STRINGSTABLEMODEL_H
#define STRINGSTABLEMODEL_H

#include "transactionitem.h"
#include "precompiled.h"

class TxTableModel: public QAbstractTableModel
{
public:
    TxTableModel(QObject* parent, QList<TransactionItem>* addresses);
    ~TxTableModel();

    QString  getTxId(int row);

    int      rowCount(const QModelIndex &parent) const;
    int      columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QList<TransactionItem>*  modeldata;
    QList<QString>           headers;
};


#endif // STRINGSTABLEMODEL_H
