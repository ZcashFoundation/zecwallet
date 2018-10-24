#include "txtablemodel.h"
#include "settings.h"
#include "utils.h"
#include "rpc.h"

TxTableModel::TxTableModel(QObject *parent)
     : QAbstractTableModel(parent) {
    headers << "Category" << "Address" << "Date/Time" << "Amount";
}

TxTableModel::~TxTableModel() {
    delete modeldata;
}

void TxTableModel::addZSentData(const QList<TransactionItem>& data) {
    delete zsTrans;
    zsTrans = new QList<TransactionItem>();
    std::copy(data.begin(), data.end(), std::back_inserter(*zsTrans));

    updateAllData();
}

void TxTableModel::addZRecvData(const QList<TransactionItem>& data) {
    delete zrTrans;
    zrTrans = new QList<TransactionItem>();
    std::copy(data.begin(), data.end(), std::back_inserter(*zrTrans));

    updateAllData();
}


void TxTableModel::addTData(const QList<TransactionItem>& data) {
    delete tTrans;
    tTrans = new QList<TransactionItem>();
    std::copy(data.begin(), data.end(), std::back_inserter(*tTrans));

    updateAllData();
}

void TxTableModel::updateAllData() {    
    auto newmodeldata = new QList<TransactionItem>();

    if (tTrans  != nullptr) std::copy( tTrans->begin(),  tTrans->end(), std::back_inserter(*newmodeldata));
    if (zsTrans != nullptr) std::copy(zsTrans->begin(), zsTrans->end(), std::back_inserter(*newmodeldata));
    if (zrTrans != nullptr) std::copy(zrTrans->begin(), zrTrans->end(), std::back_inserter(*newmodeldata));

    // Sort by reverse time
    std::sort(newmodeldata->begin(), newmodeldata->end(), [=] (auto a, auto b) {
        return a.datetime > b.datetime; // reverse sort
    });

    // And then swap out the modeldata with the new one.
    delete modeldata;
    modeldata = newmodeldata;

    dataChanged(index(0, 0), index(modeldata->size()-1, columnCount(index(0,0))-1));
    layoutChanged();
}

 int TxTableModel::rowCount(const QModelIndex&) const
 {
    if (modeldata == nullptr) return 0;
    return modeldata->size();
 }

 int TxTableModel::columnCount(const QModelIndex&) const
 {
    return headers.size();
 }


 QVariant TxTableModel::data(const QModelIndex &index, int role) const
 {
     // Align column 4 (amount) right
    if (role == Qt::TextAlignmentRole && index.column() == 3) return QVariant(Qt::AlignRight | Qt::AlignVCenter);
	
	if (role == Qt::ForegroundRole) {
        if (modeldata->at(index.row()).confirmations == 0) {
            QBrush b;
            b.setColor(Qt::red);
            return b;
		}

		// Else, just return the default brush
        QBrush b;
		b.setColor(Qt::black);
		return b;		
	}
    
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        switch (index.column()) {
        case 0: return modeldata->at(index.row()).type;
        case 1: { 
                    auto addr = modeldata->at(index.row()).address;
                    if (addr.trimmed().isEmpty()) 
                        return "(Shielded)";
                    else 
                        return addr;
                }
        case 2: return QDateTime::fromSecsSinceEpoch(modeldata->at(index.row()).datetime).toLocalTime().toString();
        case 3: {
                if (role == Qt::DisplayRole)
                    return Settings::getInstance()->getZECDisplayFormat(modeldata->at(index.row()).amount);
                else {
                    return Settings::getInstance()->getUSDFormat(modeldata->at(index.row()).amount);
                }
            }
        }
    }

    return QVariant();
 }


 QVariant TxTableModel::headerData(int section, Qt::Orientation orientation, int role) const
 {
     if (role == Qt::TextAlignmentRole && section == 3) return QVariant(Qt::AlignRight | Qt::AlignVCenter);

	 if (role == Qt::FontRole) {
		 QFont f;
		 f.setBold(true);
		 return f;
	 }

     if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
         return headers.at(section);
     }

     return QVariant();
 }

QString TxTableModel::getTxId(int row) {
    return modeldata->at(row).txid;
}