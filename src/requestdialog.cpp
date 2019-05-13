#include "requestdialog.h"
#include "ui_requestdialog.h"
#include "settings.h"
#include "addressbook.h"
#include "mainwindow.h"
#include "rpc.h"
#include "settings.h"

#include "precompiled.h"

RequestDialog::RequestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RequestDialog)
{
    ui->setupUi(this);
}

RequestDialog::~RequestDialog()
{
    delete ui;
}

void RequestDialog::setupDialog(MainWindow* main, QDialog* d, Ui_RequestDialog* req) {
    req->setupUi(d);
    Settings::saveRestore(d);

    // Setup
    req->txtMemo->setLenDisplayLabel(req->lblMemoLen);
    req->lblAmount->setText(req->lblAmount->text() + Settings::getTokenName());

    if (!main || !main->getRPC() || !main->getRPC()->getAllZAddresses() || !main->getRPC()->getAllBalances())
        return;

    for (auto addr : *main->getRPC()->getAllZAddresses()) {
        auto bal = main->getRPC()->getAllBalances()->value(addr);
        if (Settings::getInstance()->isSaplingAddress(addr)) {
            req->cmbMyAddress->addItem(addr, bal);
        }
    }
    req->cmbMyAddress->setCurrentText(main->getRPC()->getDefaultSaplingAddress());

    QIcon icon(":/icons/res/paymentreq.gif");
    req->lblPixmap->setPixmap(icon.pixmap(48, 48));
}

// Static method that shows an incoming payment request and prompts the user to pay it
void RequestDialog::showPaymentConfirmation(MainWindow* main, QString paymentURI) {
    PaymentURI payInfo = Settings::parseURI(paymentURI);
    if (!payInfo.error.isEmpty()) {
        QMessageBox::critical(main, tr("Error paying HUSH URI"), 
                tr("URI should be of the form 'hush:<addr>?amt=x&memo=y") + "\n" + payInfo.error);
        return;
    }

    QDialog d(main);
    Ui_RequestDialog req;
    setupDialog(main, &d, &req);    

    // In the view mode, all fields are read-only
    req.txtAmount->setReadOnly(true);
    req.txtFrom->setReadOnly(true);
    req.txtMemo->setReadOnly(true);

    // Payment is "to"
    req.lblAddress->setText(tr("Pay To"));

    // No Addressbook
    req.btnAddressBook->setVisible(false);

    // No "address is visible" warning
    req.lblAddressInfo->setVisible(false);

    req.txtFrom->setText(payInfo.addr);
    req.txtMemo->setPlainText(payInfo.memo);
    req.txtAmount->setText(payInfo.amt);
    req.txtAmountUSD->setText(Settings::getUSDFormat(req.txtAmount->text().toDouble()));

    req.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Pay"));

    req.lblHeader->setText(tr("You are paying a payment request. Your address will not be visible to the person requesting this payment."));

    if (d.exec() == QDialog::Accepted) {
        main->payZcashURI(paymentURI, req.cmbMyAddress->currentText());
    }
}

// Static method that shows the request dialog
void RequestDialog::showRequestZcash(MainWindow* main) {
    QDialog d(main);
    Ui_RequestDialog req;

    setupDialog(main, &d, &req);

    // Setup the Label completer for the Address
    req.txtFrom->setCompleter(main->getLabelCompleter());
    QObject::connect(req.txtFrom, &QLineEdit::textChanged, [=] (auto text) {
        auto addr = AddressBook::addressFromAddressLabel(text);
        if (!Settings::getInstance()->isSaplingAddress(addr)) {
            req.lblSaplingWarning->setText(tr("Can only request from Sapling addresses"));
            req.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        } else {
            req.lblSaplingWarning->setText("");
            req.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        }
    });

    // Wire up AddressBook button
    QObject::connect(req.btnAddressBook, &QPushButton::clicked, [=] () {
        AddressBook::open(main, req.txtFrom);
    });

    // Amount textbox
    req.txtAmount->setValidator(main->getAmountValidator());
    QObject::connect(req.txtAmount, &QLineEdit::textChanged, [=] (auto text) {
        req.txtAmountUSD->setText(Settings::getUSDFormat(text.toDouble()));
    });
    req.txtAmountUSD->setText(Settings::getUSDFormat(req.txtAmount->text().toDouble()));

    req.txtMemo->setAcceptButton(req.buttonBox->button(QDialogButtonBox::Ok));
    req.txtMemo->setLenDisplayLabel(req.lblMemoLen);
    req.txtMemo->setMaxLen(400);

    req.txtFrom->setFocus();

    if (d.exec() == QDialog::Accepted) {
        // Construct a zcash Payment URI with the data and pay it immediately.
        QString memoURI = "hush:" + req.cmbMyAddress->currentText()
                    + "?amt=" + Settings::getDecimalString(req.txtAmount->text().toDouble())
                    + "&memo=" + QUrl::toPercentEncoding(req.txtMemo->toPlainText());

        QString sendURI = "hush:" + AddressBook::addressFromAddressLabel(req.txtFrom->text()) 
                    + "?amt=0.0001"
                    + "&memo=" + QUrl::toPercentEncoding(memoURI);

        // If the disclosed address in the memo doesn't have a balance, it will automatically fallback to the default
        // sapling address
        main->payZcashURI(sendURI, req.cmbMyAddress->currentText());
    }
}
