#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_confirm.h"
#include "settings.h"
#include "rpc.h"

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
    auto balFmt = QString::number(bal, 'g', 8) + " ZEC";
    ui->sendAddressBalance->setText(balFmt);
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

    horizontalLayout_13->addWidget(Amount1);
    auto horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout_13->addItem(horizontalSpacer_4);
    sendAddressLayout->addLayout(horizontalLayout_13);

    ui->sendToLayout->insertWidget(itemNumber-1, verticalGroupBox);         

    // Delay the call to scroll to allow the scroll window to adjust
    QTimer::singleShot(10, [=] () {ui->sendToScrollArea->ensureWidgetVisible(ui->addAddressButton);});                
}

void MainWindow::removeExtraAddresses() {
    // The last one is a spacer, so ignore that
    int totalItems = ui->sendToWidgets->children().size() - 2; 

    // Clear the first field
    auto addr = ui->sendToWidgets->findChild<QLineEdit*>(QString("Address1"));
    addr->clear();
    auto amt  = ui->sendToWidgets->findChild<QLineEdit*>(QString("Amount1"));
    amt->clear();
    auto max  = ui->sendToWidgets->findChild<QCheckBox*>(QString("Max1"));
    max->setChecked(false);

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
        sumAllAmounts += settings->fees();

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

    QList<QPair<QString, double>> toAddrs;
    // For each addr/amt in the sendTo tab
    int totalItems = ui->sendToWidgets->children().size() - 2;   // The last one is a spacer, so ignore that        
    for (int i=0; i < totalItems; i++) {
        auto addr = ui->sendToWidgets->findChild<QLineEdit*>(QString("Address") % QString::number(i+1))->text().trimmed();
        auto amt  = ui->sendToWidgets->findChild<QLineEdit*>(QString("Amount")  % QString::number(i+1))->text().trimmed().toDouble();
        toAddrs.push_back(QPair<QString, double>(addr, amt));
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

    // Get all the addresses and amounts
    json allRecepients = json::array();

	// Show a confirmation dialog
	QDialog d(this);
	Ui_confirm confirm;
	confirm.setupUi(&d);

	// Remove all existing address/amt qlabels
	int totalConfirmAddrItems = confirm.sendToAddrs->children().size();
	for (int i = 0; i < totalConfirmAddrItems; i++) {
		auto addr = confirm.sendToAddrs->findChild<QLabel*>(QString("Addr") % QString::number(i+1));
		auto amt = confirm.sendToAddrs->findChild<QLabel*>(QString("Amt") % QString::number(i+1));

		delete addr;
		delete amt;
	}

	// For each addr/amt
    //std::for_each(toAddr.begin(), toAddr.end(), [&] (auto toAddr) {
    for (int i=0; i < toAddrs.size(); i++) {
        auto toAddr = toAddrs[i];

		// Construct the JSON params
        json rec = json::object();
        rec["address"]  = toAddr.first.toStdString();
        rec["amount"]   = toAddr.second;
        allRecepients.push_back(rec);

		// Add new Address widgets instead of the same one.
		{
			auto Addr = new QLabel(confirm.sendToAddrs);
			Addr->setObjectName(QString("Addr") % QString::number(i + 1));
			Addr->setWordWrap(true);
			Addr->setText(fnSplitAddressForWrap(toAddr.first));
			confirm.gridLayout->addWidget(Addr, i, 0, 1, 1);

			auto Amt = new QLabel(confirm.sendToAddrs);
			Amt->setObjectName(QString("Amt") % QString::number(i + 1));
			Amt->setText(QString::number(toAddr.second, 'g', 8) % " ZEC");
			Amt->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
			confirm.gridLayout->addWidget(Amt, i, 1, 1, 1);
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

QString MainWindow::doSendTxValidations(QString fromAddr, QList<QPair<QString, double>> toAddrs) {
    // 1. Addresses are valid format. 
    QRegExp zcexp("^zc[a-z0-9]{93}$",  Qt::CaseInsensitive);
    QRegExp zsexp("^zc[a-z0-9]{76}$",  Qt::CaseInsensitive);
    QRegExp texp("^t[a-z0-9]{34}$", Qt::CaseInsensitive);

    auto matchesAnyAddr = [&] (QString addr) {
        return  zcexp.exactMatch(addr) ||
                texp.exactMatch(addr) || 
                zsexp.exactMatch(addr);
    };


    if (!matchesAnyAddr(fromAddr)) return QString("From Address is Invalid");    

    for (auto toAddr = toAddrs.begin(); toAddr != toAddrs.end(); toAddr++) {
        if (!matchesAnyAddr(toAddr->first))
            return QString("To Address ") % toAddr->first % " is Invalid";    
    };

    return QString();
}

void MainWindow::cancelButton() {
	removeExtraAddresses();
    // Back to the balances tab
    ui->tabWidget->setCurrentIndex(0);   
}

