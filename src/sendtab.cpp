#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_confirm.h"
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
        memoButtonClicked(1);
    });
    setMemoEnabled(1, false);

    // The first Address button
    QObject::connect(ui->Address1, &QLineEdit::textChanged, [=] (auto text) {
        addressChanged(1, text);
    });

    // The first Amount button
    QObject::connect(ui->Amount1, &QLineEdit::textChanged, [=] (auto text) {
        amountChanged(1, text);
    });

    // Set up focus enter to set fees
    QObject::connect(ui->tabWidget, &QTabWidget::currentChanged, [=] (int pos) {
        if (pos == 1) {
            // Set the fees
            ui->sendTxFees->setText(QString::number(Utils::getTotalFee(), 'g', 8) %
                                    " " % Utils::getTokenName());
            ui->sendTxFeesUSD->setText(Settings::getInstance()->getUSDFormat(Utils::getTotalFee()));
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
        addressChanged(itemNumber, text);
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
        amountChanged(itemNumber, text);
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
        memoButtonClicked(itemNumber);
    });
    horizontalLayout_13->addWidget(MemoBtn1);
    setMemoEnabled(itemNumber, false);

    sendAddressLayout->addLayout(horizontalLayout_13);

    auto MemoTxt1 = new QLabel(verticalGroupBox);
    MemoTxt1->setObjectName(QString("MemoTxt") % QString::number(itemNumber));
    QFont font1;
    font1.setPointSize(10);
    MemoTxt1->setFont(font1);
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

    auto memoTxt = ui->sendToWidgets->findChild<QLabel *>(QString("MemoTxt") + QString::number(number));
    // Get the current memo if it exists
    QString currentMemo = memoTxt->text();

    // Ref to see if the button was clicked
    bool ok;
    QString newMemo = QInputDialog::getText(this, "Memo", 
                        "Please type a memo to include with the amount. The memo will be visible to the recepient",
                        QLineEdit::Normal, currentMemo, &ok);
    if (ok) {
        memoTxt->setText(newMemo);
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


void MainWindow::sendButton() {
	auto fnSplitAddressForWrap = [=] (const QString& a) -> QString {
		if (!a.startsWith("z")) return a;

		auto half = a.length() / 2;
		auto splitted = a.left(half) + "\n" + a.right(a.length() - half);
		return splitted;
	};

    // Gather the from / to addresses 
    QString fromAddr = ui->inputsCombo->currentText().split("(")[0].trimmed();


    QList<ToFields> toAddrs;
    // For each addr/amt in the sendTo tab
    int totalItems = ui->sendToWidgets->children().size() - 2;   // The last one is a spacer, so ignore that        
    for (int i=0; i < totalItems; i++) {
        QString addr = ui->sendToWidgets->findChild<QLineEdit*>(QString("Address") % QString::number(i+1))->text().trimmed();
        double  amt  = ui->sendToWidgets->findChild<QLineEdit*>(QString("Amount")  % QString::number(i+1))->text().trimmed().toDouble();        
        QString memo = ui->sendToWidgets->findChild<QLabel*>(QString("MemoTxt")  % QString::number(i+1))->text().trimmed();
        
        toAddrs.push_back( ToFields{addr, amt, memo, memo.toUtf8().toHex()} );
    }

    QString error = doSendTxValidations(fromAddr, toAddrs);
    if (!error.isEmpty()) {
        // Something went wrong, so show an error and exit
        QMessageBox msg(
            QMessageBox::Critical,
            "Transaction Error", 
            error,
            QMessageBox::Ok,
            this
        );

        msg.exec();
        // abort the Tx
        return;
    }

    auto devAddress = Utils::getDevAddr(fromAddr, toAddrs);

    // Get all the addresses and amounts
    json allRecepients = json::array();

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

	// For each addr/amt/memo, construct the JSON and also build the confirm dialog box    
    for (int i=0; i < toAddrs.size(); i++) {
        auto toAddr = toAddrs[i];

		// Construct the JSON params
        json rec = json::object();
        rec["address"]      = toAddr.addr.toStdString();
        rec["amount"]       = toAddr.amount;
        if (toAddr.addr.startsWith("z") && !toAddr.encodedMemo.trimmed().isEmpty())
            rec["memo"]     = toAddr.encodedMemo.toStdString();

        allRecepients.push_back(rec);

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
            Amt->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
            confirm.gridLayout->addWidget(AmtUSD, i*2, 2, 1, 1);            

            // Memo
            if (toAddr.addr.startsWith("z")) {
                auto Memo = new QLabel(confirm.sendToAddrs);
                Memo->setObjectName(QStringLiteral("Memo") % QString::number(i + 1));
                Memo->setText(toAddr.txtMemo);
                QFont font1;
                font1.setPointSize(10);
                Memo->setFont(font1);

                confirm.gridLayout->addWidget(Memo, (i*2)+1, 0, 1, 3);
            }
		}
    }

    // Add the dev fee to the transaction
    if (!devAddress.isEmpty() && Utils::getDevFee() > 0) {
        json devFee = json::object();
        devFee["address"] = devAddress.toStdString();
        devFee["amount"]  = Utils::getDevFee();
        allRecepients.push_back(devFee);
    }    

    // Add two rows for fees
    {
        confirm.labelMinerFee->setText("Miner Fee");
        confirm.minerFee     ->setText(Settings::getInstance()->getZECDisplayFormat(Utils::getMinerFee()));
        confirm.minerFeeUSD  ->setText(Settings::getInstance()->getUSDFormat(Utils::getMinerFee()));

        if (!devAddress.isEmpty() && Utils::getDevFee() > 0) {
            confirm.labelDevFee ->setText("Dev Fee");
            confirm.devFee      ->setText(Settings::getInstance()->getZECDisplayFormat(Utils::getDevFee()));
            confirm.devFeeUSD   ->setText(Settings::getInstance()->getUSDFormat(Utils::getDevFee()));
        } else {
            confirm.labelDevFee ->setText("");
            confirm.devFee      ->setText("");
            confirm.devFeeUSD   ->setText("");
        }
    }

    // Add sender
    json params = json::array();
    params.push_back(fromAddr.toStdString());
    params.push_back(allRecepients);

	// And show it in the confirm dialog 
	confirm.sendFrom->setText(fnSplitAddressForWrap(fromAddr));

	// Show the dialog and submit it if the user confirms
	if (d.exec() == QDialog::Accepted) {
		rpc->sendZTransaction(params, [=](const json& reply) {
			QString opid = QString::fromStdString(reply.get<json::string_t>());
			ui->statusBar->showMessage("Computing Tx: " % opid);

            // And then start monitoring the transaction
            rpc->refreshTxStatus(opid);
		});

		// Then delete the additional fields from the sendTo tab
		removeExtraAddresses();
	}	    
}

QString MainWindow::doSendTxValidations(QString fromAddr, QList<ToFields> toAddrs) {
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

    if (!matchesAnyAddr(fromAddr)) return QString("From Address is Invalid");    

    for (auto toAddr : toAddrs) {
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

