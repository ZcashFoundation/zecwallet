#include "balancestablemodel.h"
#include "addressbook.h"
#include "settings.h"


BalancesTableModel::BalancesTableModel(QObject *parent)
    : QAbstractTableModel(parent) {    
}

void BalancesTableModel::setNewData(const QMap<QString, double>* balances, 
    const QList<UnspentOutput>* outputs)
{    
    loading = false;

    int currentRows = rowCount(QModelIndex());
    // Copy over the utxos for our use
    delete utxos;
    utxos = new QList<UnspentOutput>();
    // This is a QList deep copy.
    *utxos = *outputs;

    // Process the address balances into a list
    delete modeldata;
    modeldata = new QList<std::tuple<QString, double>>();
    std::for_each(balances->keyBegin(), balances->keyEnd(), [=] (auto keyIt) {
        if (balances->value(keyIt) > 0)
            modeldata->push_back(std::make_tuple(keyIt, balances->value(keyIt)));
    });

    // And then update the data
    dataChanged(index(0, 0), index(modeldata->size()-1, columnCount(index(0,0))-1));

    // Change the layout only if the number of rows changed
    if (modeldata && modeldata->size() != currentRows)
        layoutChanged();
}

BalancesTableModel::~BalancesTableModel() {
    delete modeldata;
    delete utxos;
}

int BalancesTableModel::rowCount(const QModelIndex&) const
{
    if (modeldata == nullptr) {
        if (loading) 
            return 1;
        else 
            return 0;
    }
    return modeldata->size();
}

int BalancesTableModel::columnCount(const QModelIndex&) const
{
    return 2;
}

QVariant BalancesTableModel::data(const QModelIndex &index, int role) const
{
    if (loading) {
        if (role == Qt::DisplayRole) 
            return "Loading...";
        else
            return QVariant();
    }

    if (role == Qt::TextAlignmentRole && index.column() == 1) return QVariant(Qt::AlignRight | Qt::AlignVCenter);
    
    if (role == Qt::ForegroundRole) {
        // If any of the UTXOs for this address has zero confirmations, paint it in red
        const auto& addr = std::get<0>(modeldata->at(index.row()));
        for (auto utxo : *utxos) {
            if (utxo.address == addr && utxo.confirmations == 0) {
                QBrush b;
                b.setColor(Qt::red);
                return b;
            }
        }

        // Else, just return the default brush
        QBrush b;
        b.setColor(Qt::black);
        return b;    
    }
    
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return AddressBook::addLabelToAddress(std::get<0>(modeldata->at(index.row())));
        case 1: return Settings::getZECDisplayFormat(std::get<1>(modeldata->at(index.row())));
        }
    }

    if(role == Qt::ToolTipRole) {
        switch (index.column()) {
        case 0: return AddressBook::addLabelToAddress(std::get<0>(modeldata->at(index.row())));
        case 1: return Settings::getUSDFormat(std::get<1>(modeldata->at(index.row())));
        }
    }
    
    return QVariant();
}


QVariant BalancesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::TextAlignmentRole && section == 1) {
        return QVariant(Qt::AlignRight | Qt::AlignVCenter);
    }

    if (role == Qt::FontRole) {
        QFont f;
        f.setBold(true);
        return f;
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:                 return tr("Address");
        case 1:                 return tr("Amount");
        default:                return QVariant();
        }
    }
    return QVariant();
}

