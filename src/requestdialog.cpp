#include "requestdialog.h"
#include "ui_requestdialog.h"
#include "settings.h"
#include "addressbook.h"

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

// Static method that shows the request dialog
void RequestDialog::showRequestZcash(MainWindow* main) {
    QDialog d(main);
    Ui_RequestDialog req;
    req.setupUi(&d);
    Settings::saveRestore(&d);

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
        QString paymentURI = "zcash:" + AddressBook::addressFromAddressLabel(req.txtFrom->text())
                    + "?amt=" + Settings::getDecimalString(req.txtAmount->text().toDouble())
                    + "&memo=" + QUrl::toPercentEncoding(req.txtMemo->toPlainText());
        main->payZcashURI(paymentURI);
    }
}