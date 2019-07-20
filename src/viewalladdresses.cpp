#include "viewalladdresses.h"

ViewAllAddressesModel::ViewAllAddressesModel(QTableView *parent, QList<QString> taddrs)
     : QAbstractTableModel(parent) {
    headers << tr("Address");
    addresses = taddrs;
}


int ViewAllAddressesModel::rowCount(const QModelIndex&) const {
    return addresses.size();
}

int ViewAllAddressesModel::columnCount(const QModelIndex&) const {
    return headers.size();
}

QVariant ViewAllAddressesModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        switch(index.column()) {
            case 0: return addresses.at(index.row());
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
