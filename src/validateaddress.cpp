#include "validateaddress.h"


ValidateAddressesModel::ValidateAddressesModel(QTableView *parent, QList<QPair<QString, QString>> props)
     : QAbstractTableModel(parent) {
    headers << tr("Property") << tr("Value");
    this->props = props;
}


int ValidateAddressesModel::rowCount(const QModelIndex&) const {
    return props.size();
}

int ValidateAddressesModel::columnCount(const QModelIndex&) const {
    return headers.size();
}

QVariant ValidateAddressesModel::data(const QModelIndex &index, int role) const {
    QPair<QString, QString> p = props.at(index.row());
    if (role == Qt::DisplayRole) {
        switch(index.column()) {
            case 0: return p.first;
            case 1: return p.second;
        }
    }
    return QVariant();
}  


QVariant ValidateAddressesModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return headers.at(section);
    }

    return QVariant();
}
