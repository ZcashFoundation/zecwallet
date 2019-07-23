#include "viewalladdresses.h"
#include "settings.h"

ViewAllAddressesModel::ViewAllAddressesModel(QTableView *parent, QList<QString> taddrs, RPC* rpc)
     : QAbstractTableModel(parent) {
    headers << tr("Address") << tr("Balance (%1)").arg(Settings::getTokenName());
    addresses = taddrs;
    this->rpc = rpc;
}


int ViewAllAddressesModel::rowCount(const QModelIndex&) const {
    return addresses.size();
}

int ViewAllAddressesModel::columnCount(const QModelIndex&) const {
    return headers.size();
}

QVariant ViewAllAddressesModel::data(const QModelIndex &index, int role) const {
    QString address = addresses.at(index.row());
    if (role == Qt::DisplayRole) {
        switch(index.column()) {
            case 0: return address;
            case 1: return rpc->getAllBalances()->value(address, 0.0);
        }
    }
    return QVariant();
}  


QVariant ViewAllAddressesModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return headers.at(section);
    }

    return QVariant();
}
