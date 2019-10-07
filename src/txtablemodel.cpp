#include "txtablemodel.h"
#include "settings.h"
#include "controller.h"

TxTableModel::TxTableModel(QObject *parent)
     : QAbstractTableModel(parent) {
    headers << QObject::tr("Type") << QObject::tr("Address") << QObject::tr("Date/Time") << QObject::tr("Confirmations") << QObject::tr("Amount");
}

TxTableModel::~TxTableModel() {
    delete modeldata;
    delete tTrans;
    delete zsTrans;
    delete zrTrans;
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

bool TxTableModel::exportToCsv(QString fileName) const {
    if (!modeldata)
        return false;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
        return false;

    QTextStream out(&file);   // we will serialize the data into the file

    // Write headers
    for (int i = 0; i < headers.length(); i++) {
        out << "\"" << headers[i] << "\",";
    }
    out << "\"Memo\"";
    out << endl;
    
    // Write out each row
    for (int row = 0; row < modeldata->length(); row++) {
        for (int col = 0; col < headers.length(); col++) {
            out << "\"" << data(index(row, col), Qt::DisplayRole).toString() << "\",";
        }
        // Memo
        out << "\"" << modeldata->at(row).memo << "\"";
        out << endl;
    }

    file.close();
    return true;
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
     // Align numeric columns (confirmations, amount) right
    if (role == Qt::TextAlignmentRole && 
         (index.column() == Column::Confirmations || index.column() == Column::Amount))
        return QVariant(Qt::AlignRight | Qt::AlignVCenter);

    auto dat = modeldata->at(index.row());
    if (role == Qt::ForegroundRole) {
        if (dat.confirmations <= 0) {
            QBrush b;
            b.setColor(Qt::red);
            return b;
        }

        // Else, just return the default brush
        QBrush b;
        b.setColor(Qt::black);
        return b;        
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case Column::Type: return dat.type;
        case Column::Address: {
                    auto addr = dat.address;
                    if (addr.trimmed().isEmpty()) 
                        return "(Shielded)";
                    else 
                        return addr;
                }
        case Column::Time: return QDateTime::fromMSecsSinceEpoch(dat.datetime *  (qint64)1000).toLocalTime().toString();
        case Column::Confirmations: return QString::number(dat.confirmations);
        case Column::Amount: return Settings::getZECDisplayFormat(dat.amount);
        }
    } 

    if (role == Qt::ToolTipRole) {
        switch (index.column()) {
        case Column::Type: {
                    if (dat.memo.startsWith("zcash:")) {
                        return Settings::paymentURIPretty(Settings::parseURI(dat.memo));
                    } else {
                        return modeldata->at(index.row()).type + 
                        (dat.memo.isEmpty() ? "" : " tx memo: \"" + dat.memo + "\"");
                    }
                }
        case Column::Address: {
                    auto addr = modeldata->at(index.row()).address;
                    if (addr.trimmed().isEmpty()) 
                        return "(Shielded)";
                    else 
                        return addr;
                }
        case Column::Time: return QDateTime::fromMSecsSinceEpoch(modeldata->at(index.row()).datetime * (qint64)1000).toLocalTime().toString();
        case Column::Confirmations: return QString("%1 Network Confirmations").arg(QString::number(dat.confirmations));
        case Column::Amount: return Settings::getInstance()->getUSDFromZecAmount(modeldata->at(index.row()).amount);
        }    
    }

    if (role == Qt::DecorationRole && index.column() == 0) {
        if (!dat.memo.isEmpty()) {
            // If the memo is a Payment URI, then show a payment request icon
            if (dat.memo.startsWith("zcash:")) {
                QIcon icon(":/icons/res/paymentreq.gif");
                return QVariant(icon.pixmap(16, 16));
            } else {
                // Return the info pixmap to indicate memo
                QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);            
                return QVariant(icon.pixmap(16, 16));
                }
        } else {
            // Empty pixmap to make it align
            QPixmap p(16, 16);
            p.fill(Qt::white);
            return QVariant(p);
        }
    }

    return QVariant();
 }


 QVariant TxTableModel::headerData(int section, Qt::Orientation orientation, int role) const
 {
     if (role == Qt::TextAlignmentRole && (section == Column::Confirmations || section == Column::Amount))
        return QVariant(Qt::AlignRight | Qt::AlignVCenter);

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

QString TxTableModel::getTxId(int row) const {
    return modeldata->at(row).txid;
}

QString TxTableModel::getMemo(int row) const {
    return modeldata->at(row).memo;
}

qint64 TxTableModel::getConfirmations(int row) const {
    return modeldata->at(row).confirmations;
}

QString TxTableModel::getAddr(int row) const {
    return modeldata->at(row).address.trimmed();
}

qint64 TxTableModel::getDate(int row) const {
    return modeldata->at(row).datetime;
}

QString TxTableModel::getType(int row) const {
    return modeldata->at(row).type;
}

QString TxTableModel::getAmt(int row) const {
    return Settings::getDecimalString(modeldata->at(row).amount);
}
