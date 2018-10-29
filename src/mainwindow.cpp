#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_privkey.h"
#include "ui_about.h"
#include "ui_settings.h"
#include "ui_turnstile.h"
#include "ui_turnstileprogress.h"
#include "rpc.h"
#include "balancestablemodel.h"
#include "settings.h"
#include "utils.h"
#include "turnstile.h"
#include "senttxstore.h"

#include "precompiled.h"

using json = nlohmann::json;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	// Status Bar
	setupStatusBar();
    
	// Settings editor 
	setupSettingsModal();

    // Set up exit action
    QObject::connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    // Set up donate action
    QObject::connect(ui->actionDonate, &QAction::triggered, this, &MainWindow::donate);

    // Set up check for updates action
    QObject::connect(ui->actionCheck_for_Updates, &QAction::triggered, [=] () {
        QDesktopServices::openUrl(QUrl("https://github.com/adityapk00/zec-qt-wallet/releases"));
    });

    QObject::connect(ui->actionImport_Private_Key, &QAction::triggered, this, &MainWindow::importPrivKey);

    // Set up about action
    QObject::connect(ui->actionAbout, &QAction::triggered, [=] () {
        QDialog aboutDialog(this);
        Ui_about about;
		about.setupUi(&aboutDialog);

		QString version	= QString("Version ") % QString(APP_VERSION) % " (" % QString(__DATE__) % ")";
		about.versionLabel->setText(version);
        
        aboutDialog.exec();
    });

    // Initialize to the balances tab
    ui->tabWidget->setCurrentIndex(0);

    setupSendTab();
    setupTransactionsTab();
    setupRecieveTab();
    setupBalancesTab();
    setupTurnstileDialog();

    rpc = new RPC(new QNetworkAccessManager(this), this);
    rpc->refreshZECPrice();

    rpc->refresh(true);  // Force refresh first time
}

void MainWindow::turnstileProgress() {
    Ui_TurnstileProgress progress;
    QDialog d(this);
    progress.setupUi(&d);

    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
    progress.msgIcon->setPixmap(icon.pixmap(64, 64));

    auto fnUpdateProgressUI = [=] () {
        // Get the plan progress
        if (rpc->getTurnstile()->isMigrationPresent()) {
            auto curProgress = rpc->getTurnstile()->getPlanProgress();
            
            progress.progressTxt->setText(QString::number(curProgress.step) % QString(" / ") % QString::number(curProgress.totalSteps));
            progress.progressBar->setValue(100 * curProgress.step / curProgress.totalSteps);
            
            auto nextTxBlock = curProgress.nextBlock - Settings::getInstance()->getBlockNumber();
            
            if (curProgress.step == curProgress.totalSteps) {
                auto txt = QString("Turnstile migration finished");
                if (curProgress.hasErrors) {
                    txt = txt + ". There were some errors.\n\nYour funds are all in your wallet, so you should be able to finish moving them manually.";
                }
                progress.nextTx->setText(txt);
            } else {
                progress.nextTx->setText(QString("Next transaction in ") 
                                    % QString::number(nextTxBlock < 0 ? 0 : nextTxBlock)
                                    % " blocks\n" 
                                    % (nextTxBlock <= 0 ? "(waiting for confirmations)" : ""));
            }
            
        } else {
            progress.progressTxt->setText("");
            progress.progressBar->setValue(0);
            progress.nextTx->setText("No turnstile migration is in progress");
        }
    };

    QTimer progressTimer(this);        
    QObject::connect(&progressTimer, &QTimer::timeout, fnUpdateProgressUI);
    progressTimer.start(Utils::updateSpeed);
    fnUpdateProgressUI();
    
    auto curProgress = rpc->getTurnstile()->getPlanProgress();

    // Abort button
    if (curProgress.step != curProgress.totalSteps)
        progress.buttonBox->button(QDialogButtonBox::Discard)->setText("Abort");
    else
        progress.buttonBox->button(QDialogButtonBox::Discard)->setVisible(false);
    QObject::connect(progress.buttonBox->button(QDialogButtonBox::Discard), &QPushButton::clicked, [&] () {
        if (curProgress.step != curProgress.totalSteps) {
            auto abort = QMessageBox::warning(this, "Are you sure you want to Abort?",
                                    "Are you sure you want to abort the migration?\nAll further transactions will be cancelled.\nAll your funds are still in your wallet.",
                                    QMessageBox::Yes, QMessageBox::No);
            if (abort == QMessageBox::Yes) {
                rpc->getTurnstile()->removeFile();
                d.close();
                ui->statusBar->showMessage("Automatic Sapling turnstile migration aborted.");
            }
        }
    });

    auto accpeted = d.exec();    
    if (accpeted == QDialog::Accepted && curProgress.step == curProgress.totalSteps) {
        // Finished, so delete the file
        rpc->getTurnstile()->removeFile();
    }    
}

void MainWindow::turnstileDoMigration(QString fromAddr) {
    // Return if there is no connection
    if (rpc->getAllZAddresses() == nullptr)
        return;

    Ui_Turnstile turnstile;
    QDialog d(this);
    turnstile.setupUi(&d);

    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
    turnstile.msgIcon->setPixmap(icon.pixmap(64, 64));

    auto fnGetAllSproutBalance = [=] () {
        double bal = 0;
        for (auto addr : *rpc->getAllZAddresses()) {
            if (Settings::getInstance()->isSproutAddress(addr)) {
                bal += rpc->getAllBalances()->value(addr);
            }
        }

        return bal;
    };

    //turnstile.migrateZaddList->addItem("All Sprout z-Addrs");
    turnstile.fromBalance->setText(Settings::getInstance()->getZECUSDDisplayFormat(fnGetAllSproutBalance()));
    for (auto addr : *rpc->getAllZAddresses()) {
        if (Settings::getInstance()->isSaplingAddress(addr)) {
            turnstile.migrateTo->addItem(addr);
        } else {
            turnstile.migrateZaddList->addItem(addr);
        }
    }

    auto fnUpdateSproutBalance = [=] (QString addr) {
        double bal = 0;
        if (addr.startsWith("All")) {
            bal = fnGetAllSproutBalance();
        } else {
            bal = rpc->getAllBalances()->value(addr);
        }

        auto balTxt = Settings::getInstance()->getZECUSDDisplayFormat(bal);
        
        if (bal < Turnstile::minMigrationAmount) {
            turnstile.fromBalance->setStyleSheet("color: red;");
            turnstile.fromBalance->setText(balTxt % " [You need at least " 
                        % Settings::getInstance()->getZECDisplayFormat(Turnstile::minMigrationAmount)
                        % " for automatic migration]");
            turnstile.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        } else {
            turnstile.fromBalance->setStyleSheet("");
            turnstile.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            turnstile.fromBalance->setText(balTxt);
        }
    };

    if (!fromAddr.isEmpty())
        turnstile.migrateZaddList->setCurrentText(fromAddr);

    fnUpdateSproutBalance(turnstile.migrateZaddList->currentText());
    

    // Combo box selection event
    QObject::connect(turnstile.migrateZaddList, &QComboBox::currentTextChanged, fnUpdateSproutBalance);
        
    // Privacy level combobox
    // Num tx over num blocks
    QList<QPair<int, int>> privOptions; 
    privOptions.push_back(QPair<double, double>(3, 6));
    privOptions.push_back(QPair<double, double>(5, 10));
    privOptions.push_back(QPair<double, double>(10, 20));

    QObject::connect(turnstile.privLevel, QOverload<int>::of(&QComboBox::currentIndexChanged), [=] (auto idx) {
        // Update the fees
        turnstile.minerFee->setText(
            Settings::getInstance()->getZECUSDDisplayFormat(privOptions[idx].first * Utils::getMinerFee()));
    });

    turnstile.privLevel->addItem("Good - 3 tx over 6 blocks");
    turnstile.privLevel->addItem("Excellent - 5 tx over 10 blocks");
    turnstile.privLevel->addItem("Paranoid - 10 tx over 20 blocks");

    turnstile.buttonBox->button(QDialogButtonBox::Ok)->setText("Start");

    if (d.exec() == QDialog::Accepted) {
        auto privLevel = privOptions[turnstile.privLevel->currentIndex()];
        rpc->getTurnstile()->planMigration(
            turnstile.migrateZaddList->currentText(), 
            turnstile.migrateTo->currentText(),
            privLevel.first, privLevel.second);

        QMessageBox::information(this, "Backup your wallet.dat", 
                                    "The migration will now start. You can check progress in the File -> Sapling Turnstile menu.\n\nYOU MUST BACKUP YOUR wallet.dat NOW!\n\nNew Addresses have been added to your wallet which will be used for the migration.", 
                                    QMessageBox::Ok);
    }
}

void MainWindow::setupTurnstileDialog() {        
    // Turnstile migration
    QObject::connect(ui->actionTurnstile_Migration, &QAction::triggered, [=] () {
        // If there is current migration that is present, show the progress button
        if (rpc->getTurnstile()->isMigrationPresent())
            turnstileProgress();
        else    
            turnstileDoMigration();        
    });

}

void MainWindow::setupStatusBar() {
	// Status Bar
	loadingLabel = new QLabel();
	loadingMovie = new QMovie(":/icons/res/loading.gif");
	loadingMovie->setScaledSize(QSize(32, 16));
	loadingMovie->start();
	loadingLabel->setAttribute(Qt::WA_NoSystemBackground);
	loadingLabel->setMovie(loadingMovie);

	ui->statusBar->addPermanentWidget(loadingLabel);
	loadingLabel->setVisible(false);

	// Custom status bar menu
	ui->statusBar->setContextMenuPolicy(Qt::CustomContextMenu);
	QObject::connect(ui->statusBar, &QStatusBar::customContextMenuRequested, [=](QPoint pos) {
		auto msg = ui->statusBar->currentMessage();
		QMenu menu(this);

		if (!msg.isEmpty() && msg.startsWith(Utils::txidStatusMessage)) {
			auto txid = msg.split(":")[1].trimmed();
			menu.addAction("Copy txid", [=]() {
				QGuiApplication::clipboard()->setText(txid);
			});
			menu.addAction("View tx on block explorer", [=]() {
				QString url;
				if (Settings::getInstance()->isTestnet()) {
					url = "https://explorer.testnet.z.cash/tx/" + txid;
				}
				else {
					url = "https://explorer.zcha.in/transactions/" + txid;
				}
				QDesktopServices::openUrl(QUrl(url));
			});
		}

		menu.addAction("Refresh", [=]() {
			rpc->refresh(true);
		});
		QPoint gpos(mapToGlobal(pos).x(), mapToGlobal(pos).y() + this->height() - ui->statusBar->height());
		menu.exec(gpos);
	});

	statusLabel = new QLabel();
	ui->statusBar->addPermanentWidget(statusLabel);

	statusIcon = new QLabel();
	ui->statusBar->addPermanentWidget(statusIcon);
}

void MainWindow::setupSettingsModal() {	
	// Set up File -> Settings action
	QObject::connect(ui->actionSettings, &QAction::triggered, [=]() {
		QDialog settingsDialog(this);
		Ui_Settings settings;
		settings.setupUi(&settingsDialog);

		// Setup save sent check box
		QObject::connect(settings.chkSaveTxs, &QCheckBox::stateChanged, [=](auto checked) {
			Settings::getInstance()->setSaveZtxs(checked);
		});

		// Setup clear button
		QObject::connect(settings.btnClearSaved, &QCheckBox::clicked, [=]() {
			if (QMessageBox::warning(this, "Clear saved history?",
				"Shielded z-Address transactions are stored locally in your wallet, outside zcashd. You may delete this saved information safely any time for your privacy.\nDo you want to delete the saved shielded transactions now ?",
				QMessageBox::Yes, QMessageBox::Cancel)) {
					SentTxStore::deleteHistory();
					// Reload after the clear button so existing txs disappear
					rpc->refresh(true);
			}
		});

		// Save sent transactions
		settings.chkSaveTxs->setChecked(Settings::getInstance()->getSaveZtxs());

		// Connection Settings
		QIntValidator validator(0, 65535);
		settings.port->setValidator(&validator);

        // Load current values into the dialog		
        settings.hostname->setText(Settings::getInstance()->getHost());
        settings.port->setText(Settings::getInstance()->getPort());
        settings.rpcuser->setText(Settings::getInstance()->getUsernamePassword().split(":")[0]);
        settings.rpcpassword->setText(Settings::getInstance()->getUsernamePassword().split(":")[1]);

		// If values are coming from zcash.conf, then disable all the fields
		auto zcashConfLocation = Settings::getInstance()->getZcashdConfLocation();
		if (!zcashConfLocation.isEmpty()) {
			settings.confMsg->setText("Settings are being read from \n" + zcashConfLocation);
			settings.hostname->setEnabled(false);
			settings.port->setEnabled(false);
			settings.rpcuser->setEnabled(false);
			settings.rpcpassword->setEnabled(false);
		}
		else {
			settings.confMsg->setText("No local zcash.conf found. Please configure connection manually.");
			settings.hostname->setEnabled(true);
			settings.port->setEnabled(true);
			settings.rpcuser->setEnabled(true);
			settings.rpcpassword->setEnabled(true);
		}

		// Connection tab by default
		settings.tabWidget->setCurrentIndex(0);

		if (settingsDialog.exec() == QDialog::Accepted) {
			if (zcashConfLocation.isEmpty()) {
				// Save settings
				Settings::getInstance()->saveSettings(
					settings.hostname->text(),
					settings.port->text(),
					settings.rpcuser->text(),
					settings.rpcpassword->text());
                
                this->rpc->reloadConnectionInfo();
            }

            // Then refresh everything.			
            this->rpc->refresh(true);			
		};
	});

}

void MainWindow::donate() {
    // Set up a donation to me :)
    ui->Address1->setText(Utils::getDonationAddr(
                                Settings::getInstance()->isSaplingAddress(ui->inputsCombo->currentText())));
    ui->Address1->setCursorPosition(0);
    ui->Amount1->setText("0.01");
    ui->MemoTxt1->setText("Thanks for supporting zec-qt-wallet!");

    ui->statusBar->showMessage("Donate 0.01 " % Utils::getTokenName() % " to support zec-qt-wallet");

    // And switch to the send tab.
    ui->tabWidget->setCurrentIndex(1);
}


void MainWindow::importPrivKey() {
    QDialog d(this);
    Ui_PrivKey pui;
    pui.setupUi(&d);

    pui.helpLbl->setText(QString() %
                        "Please paste your private keys (z-Addr or t-Addr) here, one per line.\n" %
                        "The keys will be imported into your connected zcashd node");                        
    if (d.exec() == QDialog::Accepted && !pui.privKeyTxt->toPlainText().trimmed().isEmpty()) {
        auto keys = pui.privKeyTxt->toPlainText().trimmed().split("\n");
        for (int i=0; i < keys.length(); i++) {
            auto key = keys[i].trimmed();
            if (key.startsWith("S") ||
                key.startsWith("secret")) { // Z key
                rpc->importZPrivKey(key, [=] (auto) {} );
            } else {    // T Key
                rpc->importTPrivKey(key, [=] (auto) {} );
            }
        }
    }

    QMessageBox::information(this, 
        "Imported", "The keys were imported. It may be a while to rescan the blockchain with the new keys.",
        QMessageBox::Ok);
}

void MainWindow::setupBalancesTab() {
    ui->unconfirmedWarning->setVisible(false);

    // Setup context menu on balances tab
    ui->balancesTable->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->balancesTable, &QTableView::customContextMenuRequested, [=] (QPoint pos) {
        QModelIndex index = ui->balancesTable->indexAt(pos);
        if (index.row() < 0) return;

        index = index.sibling(index.row(), 0);
        auto addr = ui->balancesTable->model()->data(index).toString();

        QMenu menu(this);

        menu.addAction("Copy address", [=] () {
            QClipboard *clipboard = QGuiApplication::clipboard();
            clipboard->setText(addr);            
            ui->statusBar->showMessage("Copied to clipboard", 3 * 1000);
        });

        menu.addAction("Get private key", [=] () {
            auto fnCB = [=] (const json& reply) {
                auto privKey = QString::fromStdString(reply.get<json::string_t>());
                QDialog d(this);
                Ui_PrivKey pui;
                pui.setupUi(&d);

                pui.helpLbl->setText("Private Key:");
                pui.privKeyTxt->setPlainText(privKey);
                pui.privKeyTxt->setReadOnly(true);
                pui.privKeyTxt->selectAll();
                pui.buttonBox->button(QDialogButtonBox::Ok)->setVisible(false);

                d.exec();
            };

            if (Settings::getInstance()->isZAddress(addr)) 
                rpc->getZPrivKey(addr, fnCB);
            else
                rpc->getTPrivKey(addr, fnCB);
        });

		menu.addAction("Send from " % addr.left(40) % (addr.size() > 40 ? "..." : ""), [=]() {
			// Find the inputs combo
			for (int i = 0; i < ui->inputsCombo->count(); i++) {
				if (ui->inputsCombo->itemText(i).startsWith(addr)) {
					ui->inputsCombo->setCurrentIndex(i);
					break;
				}
			}
			
			// And switch to the send tab.
			ui->tabWidget->setCurrentIndex(1);
		});

        if (addr.startsWith("t")) {
            menu.addAction("View on block explorer", [=] () {
                QString url;
                if (Settings::getInstance()->isTestnet()) {
                    url = "https://explorer.testnet.z.cash/address/" + addr;
                } else {
                    url = "https://explorer.zcha.in/accounts/" + addr;
                }
                QDesktopServices::openUrl(QUrl(url));
            });
        }

        if (Settings::getInstance()->isSproutAddress(addr)) {
            menu.addAction("Migrate to Sapling", [=] () {
                this->turnstileDoMigration(addr);
            });
        }

        menu.exec(ui->balancesTable->viewport()->mapToGlobal(pos));            
    });
}

void MainWindow::setupTransactionsTab() {
    // Set up context menu on transactions tab
    ui->transactionsTable->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->transactionsTable, &QTableView::customContextMenuRequested, [=] (QPoint pos) {
        QModelIndex index = ui->transactionsTable->indexAt(pos);
        if (index.row() < 0) return;

        QMenu menu(this);

        auto txModel = dynamic_cast<TxTableModel *>(ui->transactionsTable->model());

        QString txid = txModel->getTxId(index.row());
        QString memo = txModel->getMemo(index.row());

        menu.addAction("Copy txid", [=] () {            
            QGuiApplication::clipboard()->setText(txid);
            ui->statusBar->showMessage("Copied to clipboard", 3 * 1000);
        });
        menu.addAction("View on block explorer", [=] () {
            QString url;
            if (Settings::getInstance()->isTestnet()) {
                url = "https://explorer.testnet.z.cash/tx/" + txid;
            } else {
                url = "https://explorer.zcha.in/transactions/" + txid;
            }
            QDesktopServices::openUrl(QUrl(url));
        });
        if (!memo.isEmpty()) {
            menu.addAction("View Memo", [=] () {
                QMessageBox::information(this, "Memo", memo, QMessageBox::Ok);
            });
        }

        menu.exec(ui->transactionsTable->viewport()->mapToGlobal(pos));        
    });
}

void MainWindow::addNewZaddr(bool sapling) {
    rpc->newZaddr(sapling, [=] (json reply) {
        QString addr = QString::fromStdString(reply.get<json::string_t>());
        // Make sure the RPC class reloads the Z-addrs for future use
        rpc->refreshAddresses();

        // Just double make sure the Z-address is still checked
        if (( sapling && ui->rdioZSAddr->isChecked()) ||
            (!sapling && ui->rdioZAddr->isChecked())) {
            ui->listRecieveAddresses->insertItem(0, addr);
            ui->listRecieveAddresses->setCurrentIndex(0);

            ui->statusBar->showMessage(QString::fromStdString("Created new zAddr") %
                                       (sapling ? "(Sapling)" : "(Sprout)"), 
                                       10 * 1000);
        }
    });
}


// Adds sapling or sprout z-addresses to the combo box. Technically, returns a
// lambda, which can be connected to the appropriate signal
std::function<void(bool)> MainWindow::addZAddrsToComboList(bool sapling) {
    return [=] (bool checked) { 
        if (checked && this->rpc->getAllZAddresses() != nullptr) { 
            auto addrs = this->rpc->getAllZAddresses();
            ui->listRecieveAddresses->clear();

            std::for_each(addrs->begin(), addrs->end(), [=] (auto addr) {
                if ( (sapling &&  Settings::getInstance()->isSaplingAddress(addr)) ||
                    (!sapling && !Settings::getInstance()->isSaplingAddress(addr)))
                    ui->listRecieveAddresses->addItem(addr);
            }); 

            // If z-addrs are empty, then create a new one.
            if (addrs->isEmpty()) {
                addNewZaddr(sapling);
            }
        } 
    };
}

void MainWindow::setupRecieveTab() {
    auto addNewTAddr = [=] () {
        rpc->newTaddr([=] (json reply) {
                QString addr = QString::fromStdString(reply.get<json::string_t>());

                // Just double make sure the T-address is still checked
                if (ui->rdioTAddr->isChecked()) {
                    ui->listRecieveAddresses->insertItem(0, addr);
                    ui->listRecieveAddresses->setCurrentIndex(0);

                    ui->statusBar->showMessage("Created new t-Addr", 10 * 1000);
                }
            });
    };

    // Connect t-addr radio button
    QObject::connect(ui->rdioTAddr, &QRadioButton::toggled, [=] (bool checked) { 
        // Whenever the T-address is selected, we generate a new address, because we don't
        // want to reuse T-addrs
        if (checked && this->rpc->getUTXOs() != nullptr) { 
            auto utxos = this->rpc->getUTXOs();
            ui->listRecieveAddresses->clear();

            std::for_each(utxos->begin(), utxos->end(), [=] (auto& utxo) {
                auto addr = utxo.address;
                if (addr.startsWith("t") && ui->listRecieveAddresses->findText(addr) < 0) {
                    ui->listRecieveAddresses->addItem(addr);
                }
            });

            addNewTAddr();
        } 
    });


    // zAddr toggle button, one for sprout and one for sapling
    QObject::connect(ui->rdioZAddr,  &QRadioButton::toggled, addZAddrsToComboList(false));
    QObject::connect(ui->rdioZSAddr, &QRadioButton::toggled, addZAddrsToComboList(true));

    // Explicitly get new address button.
    QObject::connect(ui->btnRecieveNewAddr, &QPushButton::clicked, [=] () {
        if (ui->rdioZAddr->isChecked()) {
            addNewZaddr(false); 
        } else if (ui->rdioZSAddr->isChecked()) {
            addNewZaddr(true);
        } else if (ui->rdioTAddr->isChecked()) {
            addNewTAddr();
        }
    });

    // Focus enter for the Recieve Tab
    QObject::connect(ui->tabWidget, &QTabWidget::currentChanged, [=] (int tab) {
        if (tab == 2) {
            // Switched to recieve tab, so update everything. 

            // Hide Sapling radio button if sapling is not active
            if (Settings::getInstance()->isSaplingActive()) {
                ui->rdioZSAddr->setVisible(true);    
                ui->rdioZSAddr->setChecked(true);
                ui->rdioZAddr->setText("z-Addr(Sprout)");
            } else {
                ui->rdioZSAddr->setVisible(false);    
                ui->rdioZAddr->setChecked(true);
                ui->rdioZAddr->setText("z-Addr");   // Don't use the "Sprout" label if there's no sapling
            }
            
            // And then select the first one
            ui->listRecieveAddresses->setCurrentIndex(0);
        }
    });

    // Select item in address list
    QObject::connect(ui->listRecieveAddresses, 
        QOverload<const QString &>::of(&QComboBox::currentIndexChanged), [=] (const QString& addr) {
        if (addr.isEmpty()) {
            // Draw empty stuff

            ui->txtRecieve->clear();
            ui->qrcodeDisplay->clear();
            return;
        }

        ui->txtRecieve->setPlainText(addr);       
        
        QSize sz = ui->qrcodeDisplay->size();

        QPixmap pm(sz);
        pm.fill(Qt::white);
        QPainter painter(&pm);
        
        // NOTE: At this point you will use the API to get the encoding and format you want, instead of my hardcoded stuff:
        qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(addr.toUtf8().constData(), qrcodegen::QrCode::Ecc::LOW);
        const int    s      = qr.getSize()>0?qr.getSize():1;
        const double w      = sz.width();
        const double h      = sz.height();
        const double aspect = w/h;
        const double size   = ((aspect>1.0)?h:w);
        const double scale  = size/(s+2);
		const double offset = (w - size) > 0 ? (w - size) / 2 : 0;
        // NOTE: For performance reasons my implementation only draws the foreground parts in supplied color.
        // It expects background to be prepared already (in white or whatever is preferred).
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(Qt::black));
        for(int y=0; y<s; y++) {
            for(int x=0; x<s; x++) {
                const int color=qr.getModule(x, y);  // 0 for white, 1 for black
                if(0!=color) {
                    const double rx1=(x+1)*scale+ offset, ry1=(y+1)*scale;
                    QRectF r(rx1, ry1, scale, scale);
                    painter.drawRects(&r,1);
                }
            }
        }
        
        ui->qrcodeDisplay->setPixmap(pm);
    });    

}

MainWindow::~MainWindow()
{
    delete ui;
    delete rpc;

    delete loadingMovie;
}
