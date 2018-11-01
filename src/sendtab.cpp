#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_confirm.h"
#include "ui_memodialog.h"
#include "settings.h"
#include "rpc.h"
#include "utils.h"

#include "precompiled.h"

using json = nlohmann::json;

void MainWindow::setupSendTab() {
    // Create the validator for send to/amount fields
    auto amtValidator = new QDoubleValidator(0, 21000000, 8, ui->Amount1);
    amtValidator->setNotation(QDoubleValidator::StandardNotation);
    ui->Amount1->setValidator(amtValidator);

    setDefaultPayFrom();
    
    // Send button
    QObject::connect(ui->sendTransactionButton, &QPushButton::clicked, this, &MainWindow::sendButton);

    // Cancel Button
    QObject::connect(ui->cancelSendButton, &QPushButton::clicked, this, &MainWindow::cancelButton);

    // Input Combobox current text changed
    QObject::connect(ui->inputsCombo, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
        this, &MainWindow::inputComboTextChanged);

    // Hook up add address button click
    QObject::connect(ui->addAddressButton, &QPushButton::clicked, this, &MainWindow::addAddressSection);

    // Max available Checkbox
    QObject::connect(ui->Max1, &QCheckBox::stateChanged, this, &MainWindow::maxAmountChecked);

    // The first Memo button
    QObject::connect(ui->MemoBtn1, &QPushButton::clicked, [=] () {
        this->memoButtonClicked(1);
    });
    setMemoEnabled(1, false);

    // The first Address button
    QObject::connect(ui->Address1, &QLineEdit::textChanged, [=] (auto text) {
        this->addressChanged(1, text);
    });

    // The first Amount button
    QObject::connect(ui->Amount1, &QLineEdit::textChanged, [=] (auto text) {
        this->amountChanged(1, text);
    });

    // Font for the first Memo label
    QFont f = ui->Address1->font();
    f.setPointSize(f.pointSize() - 1);
    ui->MemoTxt1->setFont(f);

    // Set up focus enter to set fees
    QObject::connect(ui->tabWidget, &QTabWidget::currentChanged, [=] (int pos) {
        if (pos == 1) {
            // Set the fees
            ui->lblMinerFee->setText(QString::number(Utils::getMinerFee(), 'g', 8) %
                                    " " % Utils::getTokenName());
            ui->lblMinerFeeUSD->setText(Settings::getInstance()->getUSDFormat(Utils::getMinerFee()));

            // Dev Fee.
            if (Utils::getDevFee() < 0.0001) {
                ui->lblDevFee->setText("");
                ui->lblDevFeeUSD->setText("");
                ui->lblDevFeeTxt->setText("");
            } else {
                ui->lblDevFee->setText(QString::number(Utils::getDevFee(), 'g', 8) %
                                    " " % Utils::getTokenName());
                ui->lblDevFeeUSD->setText(Settings::getInstance()->getUSDFormat(Utils::getDevFee()));
            }

            // Set focus to the first address box
            ui->Address1->setFocus();
        }
    });
}

void MainWindow::setDefaultPayFrom() {
    auto findMax = [=] (QString startsWith) {
        double max_amt = 0;
        int    idx     = -1;

        for (int i=0; i < ui->inputsCombo->count(); i++) {
            auto addr = ui->inputsCombo->itemText(i);
            if (addr.startsWith(startsWith)) {
                auto amt = rpc->getAllBalances()->value(addr.split("(")[0]);
                if (max_amt < amt) {
                    max_amt = amt;
                    idx = i;
                }
            }                
        }

        return idx;
    };

    // By default, select the z-address with the most balance from the inputs combo
    auto maxZ = findMax("z");
    if (maxZ >= 0) {
        ui->inputsCombo->setCurrentIndex(maxZ);                
    } else {
        auto maxT = findMax("t");
        maxT  = maxT >= 0 ? maxT : 0;
        ui->inputsCombo->setCurrentIndex(maxT);
    }
};

void MainWindow::inputComboTextChanged(const QString& text) {
    auto bal    = rpc->getAllBalances()->value(text.split("(")[0].trimmed());
    auto balFmt = QString::number(bal, 'g', 8) + " " % Utils::getTokenName();

    ui->sendAddressBalance->setText(balFmt);
    ui->sendAddressBalanceUSD->setText(Settings::getInstance()->getUSDFormat(bal));
}

    
void MainWindow::addAddressSection() {
    int itemNumber = ui->sendToWidgets->children().size() - 1;

    auto verticalGroupBox = new QGroupBox(ui->sendToWidgets);
    verticalGroupBox->setTitle(QString("Recipient ") % QString::number(itemNumber));
    verticalGroupBox->setObjectName(QString("AddressGroupBox") % QString::number(itemNumber));
    auto sendAddressLayout = new QVBoxLayout(verticalGroupBox);
    sendAddressLayout->setSpacing(6);
    sendAddressLayout->setContentsMargins(11, 11, 11, 11);

    auto horizontalLayout_12 = new QHBoxLayout();
    horizontalLayout_12->setSpacing(6);
    auto label_4 = new QLabel(verticalGroupBox);
    label_4->setText("Address");
    horizontalLayout_12->addWidget(label_4);

    auto Address1 = new QLineEdit(verticalGroupBox);
    Address1->setObjectName(QString("Address") % QString::number(itemNumber)); 
    Address1->setPlaceholderText("Address");
    QObject::connect(Address1, &QLineEdit::textChanged, [=] (auto text) {
        this->addressChanged(itemNumber, text);
    });

    horizontalLayout_12->addWidget(Address1);
    sendAddressLayout->addLayout(horizontalLayout_12);

    auto horizontalLayout_13 = new QHBoxLayout();
    horizontalLayout_13->setSpacing(6);
        
    auto label_6 = new QLabel(verticalGroupBox);
    label_6->setText("Amount");
    horizontalLayout_13->addWidget(label_6);

    auto Amount1 = new QLineEdit(verticalGroupBox);
    Amount1->setPlaceholderText("Amount");    
    Amount1->setObjectName(QString("Amount") % QString::number(itemNumber));   
    Amount1->setBaseSize(QSize(200, 0));
    // Create the validator for send to/amount fields
    auto amtValidator = new QDoubleValidator(0, 21000000, 8, Amount1);
    Amount1->setValidator(amtValidator);
    QObject::connect(Amount1, &QLineEdit::textChanged, [=] (auto text) {
        this->amountChanged(itemNumber, text);
    });

    horizontalLayout_13->addWidget(Amount1);

    auto AmtUSD1 = new QLabel(verticalGroupBox);
    AmtUSD1->setObjectName(QString("AmtUSD") % QString::number(itemNumber));   
    horizontalLayout_13->addWidget(AmtUSD1);

    auto horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalLayout_13->addItem(horizontalSpacer_4);

    auto MemoBtn1 = new QPushButton(verticalGroupBox);
    MemoBtn1->setObjectName(QString("MemoBtn") % QString::number(itemNumber));
    MemoBtn1->setText("Memo");    
    // Connect Memo Clicked button
    QObject::connect(MemoBtn1, &QPushButton::clicked, [=] () {
        this->memoButtonClicked(itemNumber);
    });
    horizontalLayout_13->addWidget(MemoBtn1);
    setMemoEnabled(itemNumber, false);

    sendAddressLayout->addLayout(horizontalLayout_13);

    auto MemoTxt1 = new QLabel(verticalGroupBox);
    MemoTxt1->setObjectName(QString("MemoTxt") % QString::number(itemNumber));
    QFont font1 = Address1->font();
    font1.setPointSize(font1.pointSize()-1);
    MemoTxt1->setFont(font1);
    MemoTxt1->setWordWrap(true);
    sendAddressLayout->addWidget(MemoTxt1);

    ui->sendToLayout->insertWidget(itemNumber-1, verticalGroupBox);         

    // Set focus into the address
    Address1->setFocus();

    // Delay the call to scroll to allow the scroll window to adjust
    QTimer::singleShot(10, [=] () {ui->sendToScrollArea->ensureWidgetVisible(ui->addAddressButton);});                
}

void MainWindow::addressChanged(int itemNumber, const QString& text) {    
    setMemoEnabled(itemNumber, text.startsWith("z"));
}

void MainWindow::amountChanged(int item, const QString& text) {
    auto usd = ui->sendToWidgets->findChild<QLabel*>(QString("AmtUSD") % QString::number(item));
    usd->setText(Settings::getInstance()->getUSDFormat(text.toDouble()));
}

void MainWindow::setMemoEnabled(int number, bool enabled) {
    auto memoBtn = ui->sendToWidgets->findChild<QPushButton*>(QString("MemoBtn") % QString::number(number));
     if (enabled) {
        memoBtn->setEnabled(true);
        memoBtn->setToolTip("");
    } else {
        memoBtn->setEnabled(false);
        memoBtn->setToolTip("Only Z addresses can have memos");
    }
}

void MainWindow::memoButtonClicked(int number) {
    // Memos can only be used with zAddrs. So check that first
    auto addr = ui->sendToWidgets->findChild<QLineEdit*>(QString("Address") + QString::number(number));
    if (!addr->text().trimmed().startsWith("z")) {
        QMessageBox msg(QMessageBox::Critical, "Memos can only be used with z Addresses",
        "The Memo field can only be used with a z Address.\n" + addr->text() + "\ndoesn't look like a z Address",
        QMessageBox::Ok, this);

        msg.exec();
        return;
    }

    // Get the current memo if it exists
    auto memoTxt = ui->sendToWidgets->findChild<QLabel *>(QString("MemoTxt") + QString::number(number));
    QString currentMemo = memoTxt->text();

    Ui_MemoDialog memoDialog;
    QDialog dialog(this);
    memoDialog.setupUi(&dialog);

    QObject::connect(memoDialog.memoTxt, &QPlainTextEdit::textChanged, [=] () {
        QString txt = memoDialog.memoTxt->toPlainText();
        memoDialog.memoSize->setText(QString::number(txt.toUtf8().size()) + "/512");

        memoDialog.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(txt.toUtf8().size() <= 512);
    });

    memoDialog.memoTxt->setPlainText(currentMemo);
    memoDialog.memoTxt->setFocus();

    if (dialog.exec() == QDialog::Accepted) {
        memoTxt->setText(memoDialog.memoTxt->toPlainText());
    }

}

void MainWindow::removeExtraAddresses() {
    // The last one is a spacer, so ignore that
    int totalItems = ui->sendToWidgets->children().size() - 2; 

    // Clear the first recepient fields
    auto addr = ui->sendToWidgets->findChild<QLineEdit*>(QString("Address1"));
    addr->clear();
    auto amt  = ui->sendToWidgets->findChild<QLineEdit*>(QString("Amount1"));
    amt->clear();
    auto amtUSD  = ui->sendToWidgets->findChild<QLabel*>(QString("AmtUSD1"));
    amtUSD->clear();
    auto max  = ui->sendToWidgets->findChild<QCheckBox*>(QString("Max1"));
    max->setChecked(false);
    auto memo = ui->sendToWidgets->findChild<QLabel*>(QString("MemoTxt1"));
    memo->clear();

    // Disable first memo btn
    setMemoEnabled(1, false);

    // Start the deletion after the first item, since we want to keep 1 send field there all there
    for (int i=1; i < totalItems; i++) {
        auto addressGroupBox = ui->sendToWidgets->findChild<QGroupBox*>(QString("AddressGroupBox") % QString::number(i+1));
            
        delete addressGroupBox;
    }    
}

void MainWindow::maxAmountChecked(int checked) {
    if (checked == Qt::Checked) {
        ui->Amount1->setReadOnly(true);
        if (rpc->getAllBalances() == nullptr) return;
           
        // Calculate maximum amount
        double sumAllAmounts = 0.0;
        // Calculate all other amounts
        int totalItems = ui->sendToWidgets->children().size() - 2;   // The last one is a spacer, so ignore that        
        // Start counting the sum skipping the first one, because the MAX button is on the first one, and we don't
        // want to include it in the sum. 
        for (int i=1; i < totalItems; i++) {
            auto amt  = ui->sendToWidgets->findChild<QLineEdit*>(QString("Amount")  % QString::number(i+1));
            sumAllAmounts += amt->text().toDouble();
        }
        sumAllAmounts += Utils::getTotalFee();

        auto addr = ui->inputsCombo->currentText().split("(")[0];

        auto maxamount  = rpc->getAllBalances()->value(addr) - sumAllAmounts;
        maxamount       = (maxamount < 0) ? 0 : maxamount;
            
        ui->Amount1->setText(QString::number(maxamount, 'g', 8));
    } else if (checked == Qt::Unchecked) {
        // Just remove the readonly part, don't change the content
        ui->Amount1->setReadOnly(false);
    }
}

// Create a Tx from the current state of the send page. 
Tx MainWindow::createTxFromSendPage() {
    Tx tx;
    // Gather the from / to addresses 
    tx.fromAddr = ui->inputsCombo->currentText().split("(")[0].trimmed();

    // For each addr/amt in the sendTo tab
    int totalItems = ui->sendToWidgets->children().size() - 2;   // The last one is a spacer, so ignore that        
    for (int i=0; i < totalItems; i++) {
        QString addr = ui->sendToWidgets->findChild<QLineEdit*>(QString("Address") % QString::number(i+1))->text().trimmed();
        double  amt  = ui->sendToWidgets->findChild<QLineEdit*>(QString("Amount")  % QString::number(i+1))->text().trimmed().toDouble();        
        QString memo = ui->sendToWidgets->findChild<QLabel*>(QString("MemoTxt")  % QString::number(i+1))->text().trimmed();
        
        tx.toAddrs.push_back( ToFields{addr, amt, memo, memo.toUtf8().toHex()} );
    }

    tx.fee = Utils::getMinerFee();
    return tx;
}

bool MainWindow::confirmTx(Tx tx, ToFields devFee) {
    auto fnSplitAddressForWrap = [=] (const QString& a) -> QString {
        if (!a.startsWith("z")) return a;

        auto half = a.length() / 2;
        auto splitted = a.left(half) + "\n" + a.right(a.length() - half);
        return splitted;
    };


    // Show a confirmation dialog
    QDialog d(this);
    Ui_confirm confirm;
    confirm.setupUi(&d);

    // Remove all existing address/amt qlabels on the confirm dialog.
    int totalConfirmAddrItems = confirm.sendToAddrs->children().size();
    for (int i = 0; i < totalConfirmAddrItems / 3; i++) {
        auto addr   = confirm.sendToAddrs->findChild<QLabel*>(QString("Addr")   % QString::number(i+1));
        auto amt    = confirm.sendToAddrs->findChild<QLabel*>(QString("Amt")    % QString::number(i+1));
        auto memo   = confirm.sendToAddrs->findChild<QLabel*>(QString("Memo")   % QString::number(i+1));
        auto amtUSD = confirm.sendToAddrs->findChild<QLabel*>(QString("AmtUSD") % QString::number(i+1));

        delete memo;
        delete addr;
        delete amt;
        delete amtUSD;
    }

    // Remove the fee labels
    delete confirm.sendToAddrs->findChild<QLabel*>("labelMinerFee");
    delete confirm.sendToAddrs->findChild<QLabel*>("minerFee");
    delete confirm.sendToAddrs->findChild<QLabel*>("minerFeeUSD");

    delete confirm.sendToAddrs->findChild<QLabel*>("labelDevFee");
    delete confirm.sendToAddrs->findChild<QLabel*>("devFee");
    delete confirm.sendToAddrs->findChild<QLabel*>("devFeeUSD");

    // For each addr/amt/memo, construct the JSON and also build the confirm dialog box    
    for (int i=0; i < tx.toAddrs.size(); i++) {
        auto toAddr = tx.toAddrs[i];

        // Add new Address widgets instead of the same one.
        {
            // Address
            auto Addr = new QLabel(confirm.sendToAddrs);
            Addr->setObjectName(QString("Addr") % QString::number(i + 1));
            Addr->setWordWrap(true);
            Addr->setText(fnSplitAddressForWrap(toAddr.addr));
            confirm.gridLayout->addWidget(Addr, i*2, 0, 1, 1);

            // Amount (ZEC)
            auto Amt = new QLabel(confirm.sendToAddrs);
            Amt->setObjectName(QString("Amt") % QString::number(i + 1));
            Amt->setText(Settings::getInstance()->getZECDisplayFormat(toAddr.amount));
            Amt->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
            confirm.gridLayout->addWidget(Amt, i*2, 1, 1, 1);

            // Amount (USD)
            auto AmtUSD = new QLabel(confirm.sendToAddrs);
            AmtUSD->setObjectName(QString("AmtUSD") % QString::number(i + 1));
            AmtUSD->setText(Settings::getInstance()->getUSDFormat(toAddr.amount));
            AmtUSD->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
            confirm.gridLayout->addWidget(AmtUSD, i*2, 2, 1, 1);            

            // Memo
            if (toAddr.addr.startsWith("z")) {
                auto Memo = new QLabel(confirm.sendToAddrs);
                Memo->setObjectName(QStringLiteral("Memo") % QString::number(i + 1));
                Memo->setText(toAddr.txtMemo);
                QFont font1 = Addr->font();
                font1.setPointSize(font1.pointSize() - 1);
                Memo->setFont(font1);
                Memo->setWordWrap(true);

                confirm.gridLayout->addWidget(Memo, (i*2)+1, 0, 1, 3);
            }
        }
    }

    // Add two rows for fees
    {
        auto i = tx.toAddrs.size() * 2;

        auto labelMinerFee = new QLabel(confirm.sendToAddrs);
        labelMinerFee->setObjectName(QStringLiteral("labelMinerFee"));
        confirm.gridLayout->addWidget(labelMinerFee, i, 0, 1, 1);
        labelMinerFee->setText("Miner Fee");

        auto minerFee = new QLabel(confirm.sendToAddrs);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        minerFee->setSizePolicy(sizePolicy);
        minerFee->setObjectName(QStringLiteral("minerFee"));
        minerFee->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        confirm.gridLayout->addWidget(minerFee, i, 1, 1, 1);
        minerFee->setText(Settings::getInstance()->getZECDisplayFormat(tx.fee));

        auto minerFeeUSD = new QLabel(confirm.sendToAddrs);
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Preferred);
        minerFeeUSD->setSizePolicy(sizePolicy1);
        minerFeeUSD->setObjectName(QStringLiteral("minerFeeUSD"));
        minerFeeUSD->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        confirm.gridLayout->addWidget(minerFeeUSD, i, 2, 1, 1);
        minerFeeUSD->setText(Settings::getInstance()->getUSDFormat(tx.fee));

        if (!devFee.addr.isEmpty()) {
            auto labelDevFee = new QLabel(confirm.sendToAddrs);
            labelDevFee->setObjectName(QStringLiteral("labelDevFee"));
            confirm.gridLayout->addWidget(labelDevFee, i+1, 0, 1, 1);
            labelDevFee ->setText("Dev Fee");
 
            auto fee = new QLabel(confirm.sendToAddrs);
            fee->setObjectName(QStringLiteral("devFee"));
            fee->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
            confirm.gridLayout->addWidget(fee, i+1, 1, 1, 1);
            fee         ->setText(Settings::getInstance()->getZECDisplayFormat(Utils::getDevFee()));

            auto devFeeUSD = new QLabel(confirm.sendToAddrs);
            devFeeUSD->setSizePolicy(sizePolicy1);
            devFeeUSD->setObjectName(QStringLiteral("devFeeUSD"));
            devFeeUSD->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
            confirm.gridLayout->addWidget(devFeeUSD, i+1, 2, 1, 1);
            devFeeUSD   ->setText(Settings::getInstance()->getUSDFormat(Utils::getDevFee()));
        } 
    }

    // And FromAddress in the confirm dialog 
    confirm.sendFrom->setText(fnSplitAddressForWrap(tx.fromAddr));

    // Show the dialog and submit it if the user confirms
    if (d.exec() == QDialog::Accepted) {        
        // Then delete the additional fields from the sendTo tab
        removeExtraAddresses();
        return true;
    } else {
        return false;
    }        
}

// Send button clicked
void MainWindow::sendButton() {
    Tx tx = createTxFromSendPage();

    QString error = doSendTxValidations(tx);
    if (!error.isEmpty()) {
        // Something went wrong, so show an error and exit
        QMessageBox msg(QMessageBox::Critical, "Transaction Error", error,
                        QMessageBox::Ok, this);

        msg.exec();

        // abort the Tx
        return;
    }

    ToFields devFee{ Utils::getDevAddr(tx), Utils::getDevFee(), "", "" };
    
    // Show a dialog to confirm the Tx
    if (confirmTx(tx, devFee)) {
        if (!devFee.addr.isEmpty())
            tx.toAddrs.push_back(devFee);

        json params = json::array();
        rpc->fillTxJsonParams(params, tx);
        std::cout << std::setw(2) << params << std::endl;

        // And send the Tx
        rpc->sendZTransaction(params, [=](const json& reply) {
            QString opid = QString::fromStdString(reply.get<json::string_t>());
            ui->statusBar->showMessage("Computing Tx: " % opid);

            // And then start monitoring the transaction
            rpc->addNewTxToWatch(tx, opid);
        });
    }        
}

QString MainWindow::doSendTxValidations(Tx tx) {
    // 1. Addresses are valid format. 
    QRegExp zcexp("^z[a-z0-9]{94}$",  Qt::CaseInsensitive);
    QRegExp zsexp("^z[a-z0-9]{77}$",  Qt::CaseInsensitive);
    QRegExp ztsexp("^ztestsapling[a-z0-9]{76}", Qt::CaseInsensitive);
    QRegExp texp("^t[a-z0-9]{34}$", Qt::CaseInsensitive);

    auto matchesAnyAddr = [&] (QString addr) {
        return  zcexp.exactMatch(addr) ||
                texp.exactMatch(addr) || 
                ztsexp.exactMatch(addr) || 
                zsexp.exactMatch(addr);
    };

    if (!matchesAnyAddr(tx.fromAddr)) return QString("From Address is Invalid");    

    for (auto toAddr : tx.toAddrs) {
        if (!matchesAnyAddr(toAddr.addr))
            return QString("Recipient Address ") % toAddr.addr % " is Invalid";
    }

    return QString();
}

void MainWindow::cancelButton() {
    removeExtraAddresses();
    // Back to the balances tab
    ui->tabWidget->setCurrentIndex(0);   
}

