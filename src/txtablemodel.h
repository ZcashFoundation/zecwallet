#ifndef STRINGSTABLEMODEL_H
#define STRINGSTABLEMODEL_H

#include "transactionitem.h"
#include "precompiled.h"

class TxTableModel: public QAbstractTableModel
{
public:
    TxTableModel(QObject* parent);    
    ~TxTableModel();

    void setNewData(QList<TransactionItem>* addresses);

    QString  getTxId(int row);

    int      rowCount(const QModelIndex &parent) const;
    int      columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QList<TransactionItem>*  modeldata  = nullptr;
    QList<QString>           headers;
};


#endif // STRINGSTABLEMODEL_H
