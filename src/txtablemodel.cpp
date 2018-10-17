#include "txtablemodel.h"
#include "utils.h"

TxTableModel::TxTableModel(QObject *parent)
     : QAbstractTableModel(parent) {
    headers << "Category" << "Address" << "Date/Time" << "Amount";
}

TxTableModel::~TxTableModel() {
    delete modeldata;
}

void TxTableModel::setNewData(QList<TransactionItem>* data)  {
    delete modeldata;
    modeldata = data;

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
        case 1: return modeldata->at(index.row()).address;
        case 2: return modeldata->at(index.row()).datetime;
        case 3: return QVariant(QString::number(modeldata->at(index.row()).amount, 'g', 8) % " " % Utils::getTokenName());
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