#include "recurring.h"

#include "mainwindow.h"
#include "controller.h"
#include "settings.h"
#include "ui_newrecurring.h"
#include "ui_recurringdialog.h"
#include "ui_recurringpayments.h"
#include "ui_recurringmultiple.h"

QString schedule_desc(Schedule s) {
    switch (s) {
    case Schedule::DAY: return "day";
    case Schedule::WEEK: return "week";
    case Schedule::MONTH: return "month";
    case Schedule::YEAR: return "5 mins";
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
RecurringPaymentInfo* Recurring::getNewRecurringFromTx(QWidget* parent, MainWindow*, Tx tx, RecurringPaymentInfo* rpi) {
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
        ui.lblAmt->setText(Settings::getUSDFromZecAmount(tx.toAddrs[0].amount));

        ui.txtMemo->setPlainText(tx.toAddrs[0].txtMemo);
        ui.txtMemo->setEnabled(false);
    }

    // Wire up ZEC/USD toggle
    QObject::connect(ui.cmbCurrency, &QComboBox::currentTextChanged, [&](QString c) {
        if (tx.toAddrs.length() < 1)
            return;

        if (c == "USD") {
            ui.lblAmt->setText(Settings::getUSDFromZecAmount(tx.toAddrs[0].amount));
        }
        else {
            ui.lblAmt->setText(Settings::getDecimalString(tx.toAddrs[0].amount));
        }
    });

    for (int i = Schedule::DAY; i <= Schedule::YEAR; i++) {
        ui.cmbSchedule->addItem("Every " + schedule_desc((Schedule)i), QVariant(i));
    }

    QObject::connect(ui.cmbSchedule, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int i) {
        qDebug() << "schedule is " << i << " current data is " << ui.cmbSchedule->currentData().toInt();
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
        ui.lblAmt->setText(rpi->getAmountPretty()); 
        ui.lblFrom->setText(rpi->fromAddr);
        ui.txtNumPayments->setText(QString::number(rpi->payments.size()));
        ui.cmbSchedule->setCurrentIndex(rpi->schedule - 1); // indexes start from 0
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
    // TODO: For testing only, year means 5 mins
    case Schedule::YEAR: nextDate = nextDate.addSecs(60 * 5); break;
    //case Schedule::YEAR: nextDate = nextDate.addYears(1); break;
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
    // Refuse to run on mainnet
    if (!Settings::getInstance()->isTestnet())
        return;

    if (!main->isPaymentsReady())
        return;

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
            executeRecurringPayment(main, rpi, { pending.first().paymentNumber });
        } else if (pending.size() > 1) {
            // There are multiple pending payments. Ask the user what they want to do with it
            // Options are: Pay latest one, Pay all or Pay none.
            processMultiplePending(rpi, main);
        }
    }
}

/**
 * Called when a particular RecurringPaymentInfo has more than one pending payment to be processed.
 * We will ask the user what he wants to do.
 */ 
void Recurring::processMultiplePending(RecurringPaymentInfo rpi, MainWindow* main) {
    Ui_RecurringPending ui;
    QDialog d(main);
    ui.setupUi(&d);
    Settings::saveRestore(&d);

    // Fill the UI
    ui.lblDesc->setText    (rpi.desc);
    ui.lblTo->setText      (rpi.toAddr);
    ui.lblSchedule->setText(rpi.getScheduleDescription());

    // Mark all the outstanding ones as pending, so it shows in the table correctly.
    for (auto& pi: rpi.payments) {
        if (pi.status == PaymentStatus::NOT_STARTED && 
            pi.date <= QDateTime::currentDateTime()) {
                pi.status = PaymentStatus::PENDING;
            }
    }

    auto model = new RecurringPaymentsListViewModel(ui.tblPending, rpi);
    ui.tblPending->setModel(model);

    // Select none by default
    ui.rNone->setChecked(true);

    // Restore the table column layout
    QSettings s;
    ui.tblPending->horizontalHeader()->restoreState(s.value("recurringmultipaymentstablevgeom").toByteArray());

    bool cancelled = (d.exec() == QDialog::Rejected);

    if (cancelled || ui.rNone->isChecked()) {
        // Update the status to skip all the pending payments
        for (auto& pi: rpi.payments) {
            if (pi.status == PaymentStatus::PENDING) {
                updatePaymentItem(rpi.getHash(), pi.paymentNumber, "", "", PaymentStatus::SKIPPED);
            }
        }
    } else if (ui.rLast->isChecked()) {
        // Update the status for all except the last to skipped
        // First, collect all the payments
        QList<int> pendingPayments;
        for (int i=0; i < rpi.payments.size(); i++) {
            if (rpi.payments[i].status == PaymentStatus::PENDING) {
                pendingPayments.append(rpi.payments[i].paymentNumber);
            }
        }

        // Update the status for all but the last one
        for (int i=0; i < pendingPayments.size()-1; i++) {
            updatePaymentItem(rpi.getHash(), pendingPayments[i], "", "", PaymentStatus::SKIPPED);
        }

        // Then execute the last one. The function will update the status
        executeRecurringPayment(main, rpi, {pendingPayments.last()});
    } else if (ui.rAll->isChecked()) {
        // Pay all of them, in a single transaction
        QList<int> pendingPayments;
        for (int i=0; i < rpi.payments.size(); i++) {
            if (rpi.payments[i].status == PaymentStatus::PENDING) {
                pendingPayments.append(rpi.payments[i].paymentNumber);
            }
        }

        // Execute all of them, and then update the status
        executeRecurringPayment(main, rpi, pendingPayments);
    }

    // Save the table column layout
    s.setValue("recurringmultipaymentstablevgeom", ui.tblPending->horizontalHeader()->saveState()); 
}

void Recurring::executeRecurringPayment(MainWindow* main, RecurringPaymentInfo rpi, QList<int> paymentNumbers) {
    // Amount is in USD or ZEC?
    auto amt = rpi.amt;
    if (rpi.currency == "USD") {
        // If there is no price, then fail the payment
        if (Settings::getInstance()->getZECPrice() == 0) {
            for (auto paymentNumber: paymentNumbers) {
                updatePaymentItem(rpi.getHash(), paymentNumber, 
                    "", QObject::tr("No ZEC price was available to convert from USD"),
                    PaymentStatus::ERROR);
            }
            return;
        }
        
        // Translate it into ZEC
        amt = rpi.amt / Settings::getInstance()->getZECPrice();
    }

    // Build a Tx
    Tx tx;
    tx.fromAddr = rpi.fromAddr;
    tx.fee      = Settings::getMinerFee();
    
    // If this is a multiple payment, we'll add up all the amounts
    if (paymentNumbers.size() > 1)
        amt *= paymentNumbers.size();

    tx.toAddrs.append(ToFields { rpi.toAddr, amt, rpi.memo, rpi.memo.toUtf8().toHex() });

    // To prevent some weird race conditions, we immediately mark the payment as paid.
    // If something goes wrong, we'll get the error callback below, and the status will be 
    // updated. If it succeeds, we'll once again update the status with the txid
    for (int paymentNumber: paymentNumbers) {
        updatePaymentItem(rpi.getHash(), paymentNumber, "", "", PaymentStatus::COMPLETED);
    }            

    // Send it off to the RPC
    doSendTx(main, tx, [=] (QString txid, QString err) {
        if (err.isEmpty()) {
            // Success, update the rpi
            for (int paymentNumber: paymentNumbers) {
                updatePaymentItem(rpi.getHash(), paymentNumber, txid, "", PaymentStatus::COMPLETED);
            }            
        } else {
            // Errored out. Bummer.
            for (int paymentNumber: paymentNumbers) {
                updatePaymentItem(rpi.getHash(), paymentNumber, "", err, PaymentStatus::ERROR);
            }
        }
    });
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
    // Make sure only 1 is showing at a time
    static bool isDialogOpen = false;

    if (isDialogOpen)
        return;

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
    QObject::connect(rd.tableView, &QTableView::doubleClicked, [=] (auto index) {
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

    isDialogOpen = true;
    d.exec();
    isDialogOpen = false;

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


int RecurringListViewModel::rowCount(const QModelIndex&) const {
    return Recurring::getInstance()->getAsList().size();
}

int RecurringListViewModel::columnCount(const QModelIndex&) const {
    return headers.size();
}

QVariant RecurringListViewModel::data(const QModelIndex &index, int role) const {
    auto rpi = Recurring::getInstance()->getAsList().at(index.row());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return rpi.getAmountPretty();
        case 1: return tr("Every ") + schedule_desc(rpi.schedule);
        case 2: return QString::number(rpi.getNumPendingPayments()) + " of " + QString::number(rpi.payments.size());
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


int RecurringPaymentsListViewModel::rowCount(const QModelIndex&) const {
    return rpi.payments.size();
}

int RecurringPaymentsListViewModel::columnCount(const QModelIndex&) const {
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
                    case PaymentStatus::PENDING:     return tr("Pending");
                    case PaymentStatus::SKIPPED:     return tr("Skipped");
                    case PaymentStatus::COMPLETED:   return tr("Paid");
                    case PaymentStatus::ERROR:       return tr("Error");
                    case PaymentStatus::UNKNOWN:     return tr("Unknown");
                    default:                         return tr("Unknown");
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
