#include "recurring.h"

#include "mainwindow.h"
#include "rpc.h"
#include "settings.h"
#include "ui_newrecurring.h"

QString schedule_desc(Schedule s) {
    switch (s) {
    case Schedule::DAY: return "day";
    case Schedule::WEEK: return "week";
    case Schedule::MONTH: return "month";
    case Schedule::YEAR: return "year";
    default: return "none";
    }
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
        if (rpi->currency == "USD") {
            ui.txtAmt->setText(Settings::getUSDFormat(rpi->amt));
        }
        else {
            ui.txtAmt->setText(Settings::getDecimalString(rpi->amt));
        }
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

// Singleton
Recurring* Recurring::instance = nullptr;