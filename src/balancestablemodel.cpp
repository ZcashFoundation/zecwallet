#include "balancestablemodel.h"

BalancesTableModel::BalancesTableModel(QObject *parent)
	: QAbstractTableModel(parent)
{	
}

void BalancesTableModel::setNewData(const QMap<QString, double>* balances, 
	const QList<UnspentOutput>* outputs)
{	
	// Copy over the utxos for our use
	delete utxos;
	utxos = new QList<UnspentOutput>();
	// This is a QList deep copy.
	*utxos = *outputs;

	// Process the address balances into a list
	delete modeldata;
	modeldata = new QList<std::tuple<QString, QString>>();
	std::for_each(balances->constKeyValueBegin(), balances->constKeyValueEnd(), [=] (auto it) {
		modeldata->push_back(std::make_tuple(it.first, QString::number(it.second, 'f')));
	});

	// And then update the data
	dataChanged(index(0, 0), index(modeldata->size()-1, columnCount(index(0,0))-1));
	layoutChanged();
}

BalancesTableModel::~BalancesTableModel() {
	delete modeldata;
	delete utxos;
}

int BalancesTableModel::rowCount(const QModelIndex&) const
{
	if (modeldata == nullptr) return 0;
	return modeldata->size();
}

int BalancesTableModel::columnCount(const QModelIndex&) const
{
	return 2;
}


QVariant BalancesTableModel::data(const QModelIndex &index, int role) const
{
	auto fnSplitAddressForWrap = [=] (const QString& a) -> QString {
		if (!a.startsWith("z")) return a;

		auto half = a.length() / 2;
		auto splitted = a.left(half) + "\n" + a.right(a.length() - half);
		return splitted;
	};

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
	
	if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
		switch (index.column()) {
		case 0: return fnSplitAddressForWrap(std::get<0>(modeldata->at(index.row())));
		case 1: return QVariant(std::get<1>(modeldata->at(index.row())) % " ZEC");
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
