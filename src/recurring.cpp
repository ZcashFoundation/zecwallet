#include "recurring.h"

#include "mainwindow.h"
#include "rpc.h"
#include "settings.h"
#include "ui_newrecurring.h"

void Recurring::showEditDialog(QWidget* parent, MainWindow* main, Tx tx) {
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
    
    ui.cmbCurrency->addItem(Settings::getTokenName());
    ui.cmbCurrency->addItem("USD");

    if (tx.toAddrs.length() > 0) {
        ui.txtToAddr->setText(tx.toAddrs[0].addr);
        ui.txtToAddr->setEnabled(false);

        ui.txtAmt->setText(Settings::getDecimalString(tx.toAddrs[0].amount));
        ui.txtAmt->setEnabled(false);

        ui.txtMemo->setPlainText(tx.toAddrs[0].txtMemo);
        ui.txtMemo->setEnabled(false);
    }

    ui.cmbSchedule->addItem("Every Day", QVariant(Schedule::DAY));
    ui.cmbSchedule->addItem("Every Week", QVariant(Schedule::WEEK));
    ui.cmbSchedule->addItem("Every Month", QVariant(Schedule::MONTH));
    ui.cmbSchedule->addItem("Every Year", QVariant(Schedule::YEAR));

    ui.txtNumPayments->setText("10");

    ui.txtDesc->setFocus();
    d.exec();
}