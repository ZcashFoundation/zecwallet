#ifndef VIEWALLADDRESSES_H
#define VIEWALLADDRESSES_H

#include "precompiled.h"
#include "controller.h"

class ViewAllAddressesModel : public QAbstractTableModel {

public:
    ViewAllAddressesModel(QTableView* parent, QList<QString> taddrs, Controller* rpc);
    ~ViewAllAddressesModel() = default;

    int      rowCount(const QModelIndex &parent) const;
    int      columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QList<QString> addresses;
    QStringList headers;    
    Controller* rpc;
};

#endif