#include "recurring.h"

#include "mainwindow.h"
#include "rpc.h"
#include "settings.h"
#include "ui_newrecurring.h"
#include "ui_recurringdialog.h"

QString schedule_desc(Schedule s) {
    switch (s) {
    case Schedule::DAY: return "day";
    case Schedule::WEEK: return "week";
    case Schedule::MONTH: return "month";
    case Schedule::YEAR: return "year";
    default: return "none";
    }
}

RecurringPaymentInfo RecurringPaymentInfo::fromJson(QJsonObject j) {
    RecurringPaymentInfo r;
    r.hashid = j["hash"].toString();
    r.desc = j["desc"].toString();
    r.fromAddr = j["from"].toString();
    r.toAddr = j["to"].toString();
    r.amt = j["amt"].toString().toDouble();
    r.memo = j["memo"].toString();
    r.currency = j["currency"].toString();
    r.schedule = (Schedule)j["schedule"].toInt();
    r.frequency = j["frequency"].toInt();
    r.numPayments = j["numpayments"].toInt();
    r.startDate = QDateTime::fromSecsSinceEpoch(j["startdate"].toString().toLongLong());
    r.completedPayments = j["completed"].toInt();

    r.history = QList<HistoryItem>();
    for (auto h : j["history"].toArray()) {
        HistoryItem item;

        item.paymentNumber = h.toObject()["paymentnumber"].toInt();
        item.date = QDateTime::fromSecsSinceEpoch(h.toObject()["date"].toString().toLongLong());
        item.txid = h.toObject()["txid"].toString();
        item.status = h.toObject()["status"].toString();

        r.history.append(item);
    }

    return r;
}

void RecurringPaymentInfo::updateHash() {
    auto val = getScheduleDescription() + fromAddr + toAddr;
    hashid = QString(QCryptographicHash::hash(val.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QJsonObject RecurringPaymentInfo::toJson() {
    QJsonArray historyJson;
    for (auto h : history) {
        historyJson.append(QJsonObject{
            {"paymentnumber", h.paymentNumber},
            {"date", QString::number(h.date.toSecsSinceEpoch())},
            {"txid", h.txid},
            {"status", h.status}
            });
    }

    auto j = QJsonObject{
        {"hash", hashid},
        {"desc", desc},
        {"from", fromAddr},
        {"to", toAddr},
        {"amt", Settings::getDecimalString(amt)},
        {"memo", memo},
        {"currency", currency},
        {"schedule", (int)schedule},
        {"frequency", frequency},
        {"numpayments", numPayments},
        {"startdate", QString::number(startDate.toSecsSinceEpoch())},
        {"completed", completedPayments},
        {"history", historyJson}
    };

    return j;
}

QString RecurringPaymentInfo::getAmountPretty() {
    return currency == "USD" ? Settings::getUSDFormat(amt) : Settings::getZECDisplayFormat(amt);
}

QString RecurringPaymentInfo::getScheduleDescription() {
    return "Pay " % getAmountPretty()
        % " every " % schedule_desc(schedule) % ", starting " % startDate.toString("yyyy-MMM-dd")
        % ", for " % QString::number(numPayments) % " payments";
}

// Returns a new Recurring payment info, created from the Tx. 
// The caller needs to take ownership of the returned object.
RecurringPaymentInfo* Recurring::getNewRecurringFromTx(QWidget* parent, MainWindow* main, Tx tx, RecurringPaymentInfo* rpi) {
    Ui_newRecurringDialog ui;
    QDialog d(parent);
    ui.setupUi(&d);
    Settings::saveRestore(&d);

    // Add all the from addresses
    auto allBalances = main->getRPC()->getAllBalances();
    for (QString addr : allBalances->keys()) {
        ui.cmbFromAddress->addItem(addr, allBalances->value(addr));
    }
    
    if (!tx.fromAddr.isEmpty()) {
        ui.cmbFromAddress->setCurrentText(tx.fromAddr);
        ui.cmbFromAddress->setEnabled(false);
    }

    ui.cmbCurrency->addItem("USD");
    ui.cmbCurrency->addItem(Settings::getTokenName());

    if (tx.toAddrs.length() > 0) {
        ui.txtToAddr->setText(tx.toAddrs[0].addr);
        ui.txtToAddr->setEnabled(false);

        // Default is USD
        ui.txtAmt->setText(Settings::getUSDFromZecAmount(tx.toAddrs[0].amount));
        ui.txtAmt->setEnabled(false);

        ui.txtMemo->setPlainText(tx.toAddrs[0].txtMemo);
        ui.txtMemo->setEnabled(false);
    }

    // Wire up ZEC/USD toggle
    QObject::connect(ui.cmbCurrency, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), [&](QString c) {
        if (tx.toAddrs.length() < 1)
            return;

        if (c == "USD") {
            ui.txtAmt->setText(Settings::getUSDFromZecAmount(tx.toAddrs[0].amount));
        }
        else {
            ui.txtAmt->setText(Settings::getDecimalString(tx.toAddrs[0].amount));
        }
    });

    for (int i = Schedule::DAY; i <= Schedule::YEAR; i++) {
        ui.cmbSchedule->addItem("Every " + schedule_desc((Schedule)i), QVariant(i));
    }

    QObject::connect(ui.cmbSchedule, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int) {
        ui.lblNextPayment->setText(getNextPaymentDate((Schedule)ui.cmbSchedule->currentData().toInt()).toString("yyyy-MMM-dd"));
    });
    ui.lblNextPayment->setText(getNextPaymentDate((Schedule)ui.cmbSchedule->currentData().toInt()).toString("yyyy-MMM-dd"));

    ui.txtNumPayments->setText("10");

    // If an existing RecurringPaymentInfo was passed in, set the UI values appropriately
    if (rpi != nullptr) {
        ui.txtDesc->setText(rpi->desc);
        ui.txtToAddr->setText(rpi->toAddr);
        ui.txtMemo->setPlainText(rpi->memo);
        
        ui.cmbCurrency->setCurrentText(rpi->currency);
        ui.txtAmt->setText(rpi->getAmountPretty()); 
        ui.cmbFromAddress->setCurrentText(rpi->fromAddr);
        ui.txtNumPayments->setText(QString::number(rpi->numPayments));
        ui.cmbSchedule->setCurrentIndex(rpi->schedule);
    }
    
    ui.txtDesc->setFocus();
    if (d.exec() == QDialog::Accepted) {
        // Construct a new Object and return it
        auto r = new RecurringPaymentInfo();
        r->desc = ui.txtDesc->text();
        r->currency = ui.cmbCurrency->currentText();
        r->numPayments = ui.txtNumPayments->text().toInt();
        r->schedule = (Schedule)ui.cmbSchedule->currentData().toInt();
        r->startDate = QDateTime::currentDateTime();
        
        updateInfoWithTx(r, tx);
        return r;
    }
    else {
        return nullptr;
    }
}

void Recurring::updateInfoWithTx(RecurringPaymentInfo* r, Tx tx) {
    r->toAddr = tx.toAddrs[0].addr;
    r->memo = tx.toAddrs[0].txtMemo;
    r->fromAddr = tx.fromAddr;
    if (r->currency.isEmpty() || r->currency == "USD") {
        r->currency = "USD";
        r->amt = tx.toAddrs[0].amount * Settings::getInstance()->getZECPrice();
    }
    else {
        r->currency = Settings::getTokenName();
        r->amt = tx.toAddrs[0].amount;
    }

    r->updateHash();
}

QDateTime Recurring::getNextPaymentDate(Schedule s) {
    auto nextDate = QDateTime::currentDateTime();

    switch (s) {
    case Schedule::DAY: nextDate = nextDate.addDays(1); break;
    case Schedule::WEEK: nextDate = nextDate.addDays(7); break;
    case Schedule::MONTH: nextDate = nextDate.addMonths(1); break;
    case Schedule::YEAR: nextDate = nextDate.addYears(1); break;
    }

    return nextDate;
}

QString Recurring::writeableFile() {
    auto filename = QStringLiteral("recurringpayments.json");

    auto dir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!dir.exists())
        QDir().mkpath(dir.absolutePath());

    if (Settings::getInstance()->isTestnet()) {
        return dir.filePath("testnet-" % filename);
    }
    else {
        return dir.filePath(filename);
    }
}

void Recurring::addRecurringInfo(const RecurringPaymentInfo& rpi) {
    if (payments.contains(rpi.hashid)) {
        payments.remove(rpi.hashid);
    }
    
    payments.insert(rpi.hashid, rpi);
    
    writeToStorage();
}


void Recurring::readFromFile() {
    QFile file(writeableFile());
    file.open(QIODevice::ReadOnly);

    QTextStream in(&file);
    auto jsondoc = QJsonDocument::fromJson(in.readAll().toUtf8());

    for (auto k : jsondoc.array()) {
        auto p = RecurringPaymentInfo::fromJson(k.toObject());
        p.updateHash();
        payments.insert(p.hashid, p);
    }
}


void Recurring::writeToStorage() {
    QFile file(writeableFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate);

    QJsonArray arr;
    for (auto k : payments.keys()) {
        arr.append(payments[k].toJson());
    }

    QTextStream out(&file);   
    out << QJsonDocument(arr).toJson();

    file.close();
}

Recurring* Recurring::getInstance() {
    if (!instance) { 
        instance = new Recurring(); 
        instance->readFromFile();
    } 
    
    return instance; 
}

// Singleton
Recurring* Recurring::instance = nullptr;


void Recurring::showRecurringDialog() {
    Ui_RecurringDialog rd;
    QDialog d;
    
    rd.setupUi(&d);
    Settings::saveRestore(&d);

    auto model = new RecurringListViewModel(rd.tableView);
    rd.tableView->setModel(model);

    d.exec();
    delete model;
}

RecurringListViewModel::RecurringListViewModel(QTableView* parent) {
    this->parent = parent;
    headers << tr("Amount") << tr("Schedule") << tr("Payments Left") << tr("To");
}


int RecurringListViewModel::rowCount(const QModelIndex &parent) const {
    return Recurring::getInstance()->getAsList().size();
}

int RecurringListViewModel::columnCount(const QModelIndex &parent) const {
    return headers.size();
}

QVariant RecurringListViewModel::data(const QModelIndex &index, int role) const {
    auto rpi = Recurring::getInstance()->getAsList().at(index.row());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return rpi.getAmountPretty();
        case 1: return tr("Every ") + schedule_desc(rpi.schedule);
        case 2: return rpi.numPayments - rpi.completedPayments; 
        case 3: return rpi.toAddr;        
        //case 4: return Recurring::getNextPaymentDate(rpi.)
        }
    }

    return QVariant();
}

QVariant RecurringListViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
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