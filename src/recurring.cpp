#include "recurring.h"

#include "mainwindow.h"
#include "rpc.h"
#include "settings.h"
#include "ui_newrecurring.h"
#include "ui_recurringdialog.h"
#include "ui_recurringpayments.h"

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

    // We create a payment info with 0 items, and then fill them in later.
    RecurringPaymentInfo r(0);
    r.desc = j["desc"].toString();
    r.fromAddr = j["from"].toString();
    r.toAddr = j["to"].toString();
    r.amt = j["amt"].toString().toDouble();
    r.memo = j["memo"].toString();
    r.currency = j["currency"].toString();
    r.schedule = (Schedule)j["schedule"].toInt();
    r.frequency = j["frequency"].toInt();
    r.startDate = QDateTime::fromSecsSinceEpoch(j["startdate"].toString().toLongLong());

    for (auto h : j["payments"].toArray()) {
        PaymentItem item;

        item.paymentNumber = h.toObject()["paymentnumber"].toInt();
        item.date   = QDateTime::fromSecsSinceEpoch(h.toObject()["date"].toString().toLongLong());
        item.txid   = h.toObject()["txid"].toString();
        item.status = (PaymentStatus)h.toObject()["status"].toInt();
        item.err    = h.toObject()["err"].toString();

        r.payments.append(item);
    }

    return r;
}

QString RecurringPaymentInfo::getHash() const {
    auto val = getScheduleDescription() + fromAddr + toAddr;

    return QString(QCryptographicHash::hash(val.toUtf8(), 
                    QCryptographicHash::Sha256).toHex());
}

QJsonObject RecurringPaymentInfo::toJson() {
    QJsonArray paymentsJson;
    for (auto h : payments) {
        paymentsJson.append(QJsonObject{
            {"paymentnumber", h.paymentNumber},
            {"date",   QString::number(h.date.toSecsSinceEpoch())},
            {"txid",   h.txid},
            {"err",    h.err},
            {"status", h.status}
        });
    }

    auto j = QJsonObject{
        {"desc", desc},
        {"from", fromAddr},
        {"to", toAddr},
        {"amt", Settings::getDecimalString(amt)},
        {"memo", memo},
        {"currency", currency},
        {"schedule", (int)schedule},
        {"frequency", frequency},
        {"startdate", QString::number(startDate.toSecsSinceEpoch())},
        {"payments", paymentsJson}
    };

    return j;
}

QString RecurringPaymentInfo::getAmountPretty() const {
    return currency == "USD" ? Settings::getUSDFormat(amt) : Settings::getZECDisplayFormat(amt);
}

QString RecurringPaymentInfo::getScheduleDescription() const {
    return "Pay " % getAmountPretty()
        % " every " % schedule_desc(schedule) % ", starting " % startDate.toString("yyyy-MMM-dd")
        % ", for " % QString::number(payments.size()) % " payments";
}

/**
 * Get the date/time when the next payment is due
 */
QDateTime RecurringPaymentInfo::getNextPayment() const {
    for (auto item : payments) {
        if (item.status == PaymentStatus::NOT_STARTED)
            return item.date;
    }

    return QDateTime::fromSecsSinceEpoch(0);
} 

/**
 * Counts the number of payments that haven't been started yet
 */ 
int RecurringPaymentInfo::getNumPendingPayments() const {
    int count = 0;
    for (auto item : payments) {
        if (item.status == PaymentStatus::NOT_STARTED) {
            count++;
        }
    }

    return count;
}

// Returns a new Recurring payment info, created from the Tx. 
// The caller needs to take ownership of the returned object.
RecurringPaymentInfo* Recurring::getNewRecurringFromTx(QWidget* parent, MainWindow* main, Tx tx, RecurringPaymentInfo* rpi) {
    Ui_newRecurringDialog ui;
    QDialog d(parent);
    ui.setupUi(&d);
    Settings::saveRestore(&d);

    if (!tx.fromAddr.isEmpty()) {
        ui.lblFrom->setText(tx.fromAddr);
    }

    ui.cmbCurrency->addItem("USD");
    ui.cmbCurrency->addItem(Settings::getTokenName());

    if (tx.toAddrs.length() > 0) {
        ui.lblTo->setText(tx.toAddrs[0].addr);

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
        ui.lblTo->setText(rpi->toAddr);
        ui.txtMemo->setPlainText(rpi->memo);
        
        ui.cmbCurrency->setCurrentText(rpi->currency);
        ui.txtAmt->setText(rpi->getAmountPretty()); 
        ui.lblFrom->setText(rpi->fromAddr);
        ui.txtNumPayments->setText(QString::number(rpi->payments.size()));
        ui.cmbSchedule->setCurrentIndex(rpi->schedule);
    }
    
    ui.txtDesc->setFocus();
    if (d.exec() == QDialog::Accepted) {
        // Construct a new Object and return it
        auto numPayments = ui.txtNumPayments->text().toInt();
        auto r = new RecurringPaymentInfo(numPayments);
        r->desc = ui.txtDesc->text();
        r->currency = ui.cmbCurrency->currentText();        
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

    // Make sure that the number of payments is properly listed in the array
    assert(r->payments.size() == r->payments.size());

    // Update the payment dates
    r->payments[0].date = r->startDate;
    r->payments[0].status = PaymentStatus::NOT_STARTED;
    for (int i = 1; i < r->payments.size(); i++) {
        r->payments[i].date = getNextPaymentDate(r->schedule, r->payments[i-1].date);
        r->payments[i].status = PaymentStatus::NOT_STARTED;
    }
}

/**
 * Given a schedule and an optional previous date, calculate the next payment date/
 * If there is no previous date, it is assumed to be the current DateTime
 */ 
QDateTime Recurring::getNextPaymentDate(Schedule s, QDateTime start) {
    QDateTime nextDate = start;

    switch (s) {
    case Schedule::DAY: nextDate = nextDate.addDays(1); break;
    case Schedule::WEEK: nextDate = nextDate.addDays(7); break;
    case Schedule::MONTH: nextDate = nextDate.addMonths(1); break;
    // TODO: For teesting only
    //case Schedule::YEAR: nextDate = nextDate.addSecs(60); break;
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
    if (payments.contains(rpi.getHash())) {
        payments.remove(rpi.getHash());
    }
    
    payments.insert(rpi.getHash(), rpi);
    
    writeToStorage();
}

void Recurring::removeRecurringInfo(QString hash) {
    if (!payments.contains(hash)) {
        qDebug() << "Hash not found:" << hash << " in " << payments.keys();
        return;
    }
    
    payments.remove(hash);
    
    writeToStorage();
}


void Recurring::readFromStorage() {
    QFile file(writeableFile());
    file.open(QIODevice::ReadOnly);

    QTextStream in(&file);
    auto jsondoc = QJsonDocument::fromJson(in.readAll().toUtf8());

    payments.clear();

    for (auto k : jsondoc.array()) {
        auto p = RecurringPaymentInfo::fromJson(k.toObject());
        payments.insert(p.getHash(), p);
    }
}


void Recurring::writeToStorage() {
    QFile file(writeableFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate);

    QJsonArray arr;
    for (auto v : payments.values()) {
        arr.append(v.toJson());
    }

    QTextStream out(&file);   
    out << QJsonDocument(arr).toJson();

    file.close();
}

/**
 * Lookup the recurring payment info with the given hash, and update
 * a payment made
 **/
bool Recurring::updatePaymentItem(QString hash, int paymentNumber, 
                    QString txid, QString err, PaymentStatus status) {
    if (!payments.contains(hash)) {
        return false;
    }

    payments[hash].payments[paymentNumber].date   = QDateTime::currentDateTime();
    payments[hash].payments[paymentNumber].txid   = txid;
    payments[hash].payments[paymentNumber].err    = err;
    payments[hash].payments[paymentNumber].status = status;

    // Upda teht file on disk
    writeToStorage();

    // Then read it back to refresh the hashes
    readFromStorage();

    return true;
}

Recurring* Recurring::getInstance() {
    if (!instance) { 
        instance = new Recurring(); 
        instance->readFromStorage();
    } 
    
    return instance; 
}

// Singleton
Recurring* Recurring::instance = nullptr;

/**
 * Main worker method that will go over all the recurring paymets and process any pending ones
 */
void Recurring::processPending(MainWindow* main) {
    qDebug() << "Processing payments";

    // For each recurring payment
    for (auto rpi: payments.values()) {
        // Collect all pending payments that are past due
        QList<RecurringPaymentInfo::PaymentItem> pending;

        for (auto pi: rpi.payments) {
            if (pi.status == PaymentStatus::NOT_STARTED && 
                pi.date <= QDateTime::currentDateTime()) {
                    pending.append(pi);
                }
        }

        qDebug() << "Found " << pending.size() << "Pending Payments";

        // If there is only 1 pending payment, then we don't have to do anything special.
        // Just process it
        if (pending.size() == 1) {
            auto pi = pending[0];

            // Amount is in USD or ZEC?
            auto amt = rpi.amt;
            if (rpi.currency == "USD") {
                // If there is no price, then fail the payment
                if (Settings::getInstance()->getZECPrice() == 0) {
                    updatePaymentItem(rpi.getHash(), pi.paymentNumber, 
                    "", QObject::tr("No ZEC price was available to convert from USD"),
                    PaymentStatus::ERROR);
                        break;
                }
                
                // Translate it into ZEC
                amt = rpi.amt / Settings::getInstance()->getZECPrice();
            }

            // Build a Tx
            Tx tx;
            tx.fromAddr = rpi.fromAddr;
            tx.fee      = Settings::getMinerFee();
            tx.toAddrs  = { ToFields { rpi.toAddr, amt, rpi.memo, rpi.memo.toUtf8().toHex() } };

            doSendTx(main, tx, [=] (QString txid, QString err) {
                if (err.isEmpty()) {
                    // Success, update the rpi
                    updatePaymentItem(rpi.getHash(), pi.paymentNumber, txid, "", PaymentStatus::COMPLETED);
                } else {
                    // Errored out. Bummer.
                    updatePaymentItem(rpi.getHash(), pi.paymentNumber, "", err, PaymentStatus::ERROR);
                }
            });
        }
    }
}

/**
 * Execute a send Tx
 */ 
void Recurring::doSendTx(MainWindow* mainwindow, Tx tx, std::function<void(QString, QString)> cb) {
    mainwindow->getRPC()->executeTransaction(tx, [=] (QString opid) {
            mainwindow->ui->statusBar->showMessage(QObject::tr("Computing Recurring Tx: ") % opid);
        },
        [=] (QString /*opid*/, QString txid) { 
            mainwindow->ui->statusBar->showMessage(Settings::txidStatusMessage + " " + txid);
            cb(txid, "");
        },
        [=] (QString opid, QString errStr) {
            mainwindow->ui->statusBar->showMessage(QObject::tr(" Tx ") % opid % QObject::tr(" failed"), 15 * 1000);
            cb("", errStr);
        });
    
}

/**
 * Show the list of configured recurring payments
 */ 
void Recurring::showRecurringDialog(MainWindow* parent) {
    Ui_RecurringDialog rd;
    QDialog d(parent);
    
    rd.setupUi(&d);
    Settings::saveRestore(&d);

    auto model = new RecurringListViewModel(rd.tableView);
    rd.tableView->setModel(model);

    // Restore the table column layout
    QSettings s;
    rd.tableView->horizontalHeader()->restoreState(s.value("recurringtablegeom").toByteArray());

    // Function to show the history and pending payments for a particular recurring payment
    auto showPayments = [=, &d] (const RecurringPaymentInfo& rpi) {
        Ui_RecurringPayments p;
        QDialog pd(&d);

        p.setupUi(&pd);
        Settings::saveRestore(&pd);

        auto model = new RecurringPaymentsListViewModel(p.tableView, rpi);
        p.tableView->setModel(model);

        p.tableView->setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(p.tableView, &QTableView::customContextMenuRequested, [=, &pd] (QPoint pos) {
            QModelIndex index = p.tableView->indexAt(pos);
            if (index.row() < 0 || index.row() >= rpi.payments.size()) return;

            int paymentNumber = index.row();
            auto txid = rpi.payments[paymentNumber].txid;
            QMenu menu(parent);

            if (!txid.isEmpty()) {
                menu.addAction(QObject::tr("View on block explorer"), [=] () {
                    QString url;
                    if (Settings::getInstance()->isTestnet()) {
                        url = "https://explorer.testnet.z.cash/tx/" + txid;
                    } else {
                        url = "https://explorer.zcha.in/transactions/" + txid;
                    }
                    QDesktopServices::openUrl(QUrl(url));
                });
            }

            auto err =  rpi.payments[paymentNumber].err;
            if (!err.isEmpty()) {
                menu.addAction(QObject::tr("View Error"), [=, &pd] () {
                    QMessageBox::information(&pd, QObject::tr("Reported Error"), "\"" + err + "\"", QMessageBox::Ok);
                });
            }

            menu.exec(p.tableView->viewport()->mapToGlobal(pos));
        });

        // Restore the table column layout
        QSettings s;
        p.tableView->horizontalHeader()->restoreState(s.value("recurringpaymentstablevgeom").toByteArray());
    
        pd.exec();

        // Save the table column layout
        s.setValue("recurringpaymentstablevgeom", p.tableView->horizontalHeader()->saveState()); 
    };

    // View Button
    QObject::connect(rd.btnView, &QPushButton::clicked, [=] () {
        auto selectedRows = rd.tableView->selectionModel()->selectedRows();
        if (selectedRows.size() == 1) {
            auto rpi = Recurring::getInstance()->getAsList()[selectedRows[0].row()];
            showPayments(rpi);               
        }
    });

    // Double Click
    QObject::connect(rd.tableView, &QTableView::doubleClicked, [=, &d] (auto index) {
        auto rpi = Recurring::getInstance()->getAsList()[index.row()];
        showPayments(rpi);           
    });

    // Delete button
    QObject::connect(rd.btnDelete, &QPushButton::clicked, [=, &d]() {
        auto selectedRows = rd.tableView->selectionModel()->selectedRows();
        if (selectedRows.size() == 1) {
            auto rpi = Recurring::getInstance()->getAsList()[selectedRows[0].row()];
            if (QMessageBox::warning(&d, QObject::tr("Are you sure you want to delete the recurring payment?"), 
                QObject::tr("Are you sure you want to delete the recurring payment?") + "\n" + 
                QObject::tr("All future payments will be cancelled."),
                QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
                    Recurring::getInstance()->removeRecurringInfo(rpi.getHash());
                }
        }
    });

    d.exec();

    // Save the table column layout
    s.setValue("recurringtablegeom", rd.tableView->horizontalHeader()->saveState());

    delete model;
}

/**
 * Model for  List of recurring payments
 */ 
RecurringListViewModel::RecurringListViewModel(QTableView* parent) {
    this->parent = parent;
    headers << tr("Amount") << tr("Schedule") << tr("Payments Left") 
            << tr("Next Payment") << tr("To");
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
        case 2: return rpi.getNumPendingPayments() + " of " + rpi.payments.size(); 
        case 3: { 
            auto n = rpi.getNextPayment();
            if (n.toSecsSinceEpoch() == 0) return tr("None"); else return n;
        }
        case 4: return rpi.toAddr;        
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

/**
 * Model for history of payments for a single recurring payment
 */
RecurringPaymentsListViewModel::RecurringPaymentsListViewModel(QTableView* parent, RecurringPaymentInfo rpi) {
    this->parent = parent;
    this->rpi = rpi;
    headers << tr("Date") << tr("Status") << tr("Txid");
}


int RecurringPaymentsListViewModel::rowCount(const QModelIndex &parent) const {
    return rpi.payments.size();
}

int RecurringPaymentsListViewModel::columnCount(const QModelIndex &parent) const {
    return headers.size();
}

QVariant RecurringPaymentsListViewModel::data(const QModelIndex &index, int role) const {
    auto item = rpi.payments[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return item.date;
            case 1: {
                switch(item.status) {
                    case PaymentStatus::NOT_STARTED: return tr("Not due yet");
                    case PaymentStatus::SKIPPED:     return tr("Skipped");
                    case PaymentStatus::COMPLETED:   return tr("Paid");
                    case PaymentStatus::ERROR:       return tr("Error");
                    case PaymentStatus::UNKNOWN:     return tr("Unknown");
                }
            }
            case 2: return item.txid;
        }
    }

    if (role == Qt::ToolTipRole && !item.err.isEmpty()) {
        return item.err;
    }

    return QVariant();
}

QVariant RecurringPaymentsListViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
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