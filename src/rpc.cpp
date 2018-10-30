#include "rpc.h"
#include "utils.h"
#include "settings.h"
#include "senttxstore.h"
#include "turnstile.h"

using json = nlohmann::json;

RPC::RPC(QNetworkAccessManager* client, MainWindow* main) {
	this->restclient = client;
	this->main = main;
	this->ui = main->ui;

    this->turnstile = new Turnstile(this, main);

    // Setup balances table model
    balancesTableModel = new BalancesTableModel(main->ui->balancesTable);
    main->ui->balancesTable->setModel(balancesTableModel);
    main->ui->balancesTable->setColumnWidth(0, 300);

    // Setup transactions table model
    transactionsTableModel = new TxTableModel(ui->transactionsTable);
    main->ui->transactionsTable->setModel(transactionsTableModel);
    main->ui->transactionsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    main->ui->transactionsTable->setColumnWidth(1, 350);
    main->ui->transactionsTable->setColumnWidth(2, 200);
    main->ui->transactionsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

	reloadConnectionInfo();

    // Set up timer to refresh Price
    priceTimer = new QTimer(main);
    QObject::connect(priceTimer, &QTimer::timeout, [=]() {
        refreshZECPrice();
    });
    priceTimer->start(Utils::priceRefreshSpeed);  // Every hour

	// Set up a timer to refresh the UI every few seconds
	timer = new QTimer(main);
	QObject::connect(timer, &QTimer::timeout, [=]() {
		refresh();
	});
	timer->start(Utils::updateSpeed);    

    // Set up the timer to watch for tx status
    txTimer = new QTimer(main);
    QObject::connect(txTimer, &QTimer::timeout, [=]() {
        watchTxStatus();
    });
    // Start at every 10s. When an operation is pending, this will change to every second
    txTimer->start(Utils::updateSpeed);  

}

RPC::~RPC() {
    delete timer;
    delete txTimer;

    delete transactionsTableModel;
    delete balancesTableModel;
    delete turnstile;

    delete utxos;
    delete allBalances;
    delete zaddresses;

    delete restclient;
}

void RPC::reloadConnectionInfo() {
	// Reset for any errors caused.
	firstTime = true;
		 
    QUrl myurl;
    myurl.setScheme("http"); //https also applicable
    myurl.setHost(Settings::getInstance()->getHost());
    myurl.setPort(Settings::getInstance()->getPort().toInt());

    request.setUrl(myurl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    
    QString headerData = "Basic " + Settings::getInstance()->getUsernamePassword().toLocal8Bit().toBase64();
    request.setRawHeader("Authorization", headerData.toLocal8Bit());
}

void RPC::doRPC(const json& payload, const std::function<void(json)>& cb) {
    QNetworkReply *reply = restclient->post(request, QByteArray::fromStdString(payload.dump()));

    QObject::connect(reply, &QNetworkReply::finished, [=] {
        reply->deleteLater();
        
        if (reply->error() != QNetworkReply::NoError) {
            auto parsed = json::parse(reply->readAll(), nullptr, false);
            if (!parsed.is_discarded() && !parsed["error"]["message"].is_null()) {
                handleConnectionError(QString::fromStdString(parsed["error"]["message"]));    
            } else {
                handleConnectionError(reply->errorString());
            }
            
            return;
        } 
        
        auto parsed = json::parse(reply->readAll(), nullptr, false);
        if (parsed.is_discarded()) {
            handleConnectionError("Unknown error");
        }
        
        cb(parsed["result"]);        
    });
}

void RPC::getZAddresses(const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_listaddresses"},
    };

    doRPC(payload, cb);
}

void RPC::getTransparentUnspent(const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "listunspent"},
        {"params", {0}}             // Get UTXOs with 0 confirmations as well.
    };

    doRPC(payload, cb);
}

void RPC::getZUnspent(const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_listunspent"},
        {"params", {0}}             // Get UTXOs with 0 confirmations as well.
    };

    doRPC(payload, cb);
}

void RPC::newZaddr(bool sapling, const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_getnewaddress"},
        {"params", { sapling ? "sapling" : "sprout" }},
    };
    
    doRPC(payload, cb);
}

void RPC::newTaddr(const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "getnewaddress"},
    };

    doRPC(payload, cb);
}

void RPC::getZPrivKey(QString addr, const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_exportkey"},
        {"params", { addr.toStdString() }},
    };
    
    doRPC(payload, cb);
}

void RPC::getTPrivKey(QString addr, const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "dumpprivkey"},
        {"params", { addr.toStdString() }},
    };
    
    doRPC(payload, cb);
}

void RPC::importZPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_importkey"},
        {"params", { addr.toStdString(), (rescan? "yes" : "no") }},
    };

    qDebug() << QString::fromStdString(payload.dump());
    
    doSendRPC(payload, cb);
}


void RPC::importTPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "importprivkey"},
        {"params", { addr.toStdString(), (rescan? "yes" : "no") }},
    };
    
    qDebug() << QString::fromStdString(payload.dump());
    doSendRPC(payload, cb);
}


void RPC::getBalance(const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_gettotalbalance"},
        {"params", {0}}             // Get Unconfirmed balance as well.
    };

    doRPC(payload, cb);
}

void RPC::getTransactions(const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "listtransactions"}
    };

    doRPC(payload, cb);
}

void RPC::doSendRPC(const json& payload, const std::function<void(json)>& cb, const std::function<void(QString)>& err) {
	QNetworkReply *reply = restclient->post(request, QByteArray::fromStdString(payload.dump()));

	QObject::connect(reply, &QNetworkReply::finished, [=] {
		reply->deleteLater();

		if (reply->error() != QNetworkReply::NoError) {
			auto parsed = json::parse(reply->readAll(), nullptr, false);
			if (!parsed.is_discarded() && !parsed["error"]["message"].is_null()) {
				err(QString::fromStdString(parsed["error"]["message"]));
			}
			else {
				err(reply->errorString());
			}

			return;
		}

		auto parsed = json::parse(reply->readAll(), nullptr, false);
		if (parsed.is_discarded()) {
			err("Unknown error");
		}

		cb(parsed["result"]);
	});
}

// Default implementation of a Send RPC that default shows an error message box with the error. 
void RPC::doSendRPC(const json& payload, const std::function<void(json)>& cb) {
	doSendRPC(payload, cb, [=](auto error) { this->handleTxError(error); });
}

void RPC::sendZTransaction(json params, const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_sendmany"},
        {"params", params}
    };

    doSendRPC(payload, cb);
}

void RPC::handleConnectionError(const QString& error) {
    if (error.isNull()) return;

    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);            
    main->statusIcon->setPixmap(icon.pixmap(16, 16));
    main->statusLabel->setText("No Connection");

    if (firstTime) {
        this->firstTime = false;            

        QMessageBox msg(main);
        msg.setIcon(QMessageBox::Icon::Critical); 
        msg.setWindowTitle("Connection Error");
        
        QString explanation;
        if (error.contains("authentication", Qt::CaseInsensitive)) {
            explanation = QString() 
                        % "\n\nThis is most likely because of misconfigured rpcuser/rpcpassword. "
                        % "zcashd needs the following options set in ~/.zcash/zcash.conf\n\n"
                        % "rpcuser=<someusername>\n"
                        % "rpcpassword=<somepassword>\n"
                        % "\nIf you're connecting to a remote note, you can change the username/password in the "
                        % "File->Settings menu.";
        } else if (error.contains("connection refused", Qt::CaseInsensitive)) {
			auto confLocation = Settings::getInstance()->getZcashdConfLocation();
			if (confLocation.isEmpty()) {
				explanation = QString()
					% "\n\nA zcash.conf was not found on this machine. If you are connecting to a remote/non-standard node " 
					% "please set the host/port and user/password in the File->Settings menu.";
			}
			else {
				explanation = QString()
					% "\n\nA zcash.conf was found at\n" % confLocation
					% "\nbut we can't connect to zcashd. Is rpcuser=<user> and rpcpassword=<pass> set in the zcash.conf file?";
			}
        } else if (error.contains("internal server error", Qt::CaseInsensitive) ||
                   error.contains("rewinding", Qt::CaseInsensitive) || 
                   error.contains("loading", Qt::CaseInsensitive)) {
            explanation = QString()
                        % "\n\nIf you just started zcashd, then it's still loading and you might have to wait a while. If zcashd is ready, then you've run into  "
                        % "something unexpected, and might need to file a bug report here: https://github.com/adityapk00/zec-qt-wallet/issues";
        } else {
            explanation = QString()
                        % "\n\nThis is most likely an internal error. Something unexpected happened. "
                        % "You might need to file a bug report here: https://github.com/adityapk00/zec-qt-wallet/issues";
        }

        msg.setText("There was a network connection error. The error was: \n\n" 
                    + error + explanation);        

        msg.exec();      
        return;
    } 
}

void RPC::handleTxError(const QString& error) {
    if (error.isNull()) return;

    QMessageBox msg(main);
    msg.setIcon(QMessageBox::Icon::Critical); 
    msg.setWindowTitle("Transaction Error");
    
    msg.setText("There was an error sending the transaction. The error was: \n\n" 
                + error);        

    msg.exec();
}


// Build the RPC JSON Parameters for this tx (with the dev fee included if applicable)
void RPC::fillTxJsonParams(json& params, Tx tx) {   
    Q_ASSERT(params.is_array());
    // Get all the addresses and amounts
    json allRecepients = json::array();

    // For each addr/amt/memo, construct the JSON and also build the confirm dialog box    
    for (int i=0; i < tx.toAddrs.size(); i++) {
        auto toAddr = tx.toAddrs[i];

		// Construct the JSON params
        json rec = json::object();
        rec["address"]      = toAddr.addr.toStdString();
        rec["amount"]       = QString::number(toAddr.amount, 'f', 8).toDouble();        // Force it through string for rounding
        if (toAddr.addr.startsWith("z") && !toAddr.encodedMemo.trimmed().isEmpty())
            rec["memo"]     = toAddr.encodedMemo.toStdString();

        allRecepients.push_back(rec);
    }

    // Add sender    
    params.push_back(tx.fromAddr.toStdString());
    params.push_back(allRecepients);
}


// Refresh received z txs by calling z_listreceivedbyaddress/gettransaction
void RPC::refreshReceivedZTrans(QList<QString> zaddrs) {

    // We'll only refresh the received Z txs if settings allows us.
    if (!Settings::getInstance()->getSaveZtxs()) {
        QList<TransactionItem> emptylist;
        transactionsTableModel->addZRecvData(emptylist);
        return;
    }
        
    // This method is complicated because z_listreceivedbyaddress only returns the txid, and 
    // we have to make a follow up call to gettransaction to get details of that transaction. 
    // Additionally, it has to be done in batches, because there are multiple z-Addresses, 
    // and each z-Addr can have multiple received txs. 

    // 1. For each z-Addr, get list of received txs    
    getBatchRPC<QString>(zaddrs,
        [=] (QString zaddr) {
            json payload = {
                {"jsonrpc", "1.0"},
                {"id", "z_lrba"},
                {"method", "z_listreceivedbyaddress"},
                {"params", {zaddr.toStdString(), 0}}      // Accept 0 conf as well.
            };

            return payload;
        },          
        [=] (QMap<QString, json>* zaddrTxids) {
            // Process all txids, removing duplicates. This can happen if the same address
            // appears multiple times in a single tx's outputs.
            QSet<QString> txids;
            QMap<QString, QString> memos;
            for (auto it = zaddrTxids->constBegin(); it != zaddrTxids->constEnd(); it++) {
                auto zaddr = it.key();
                for (auto& i : it.value().get<json::array_t>()) {   
                    // Filter out change txs
                    if (! i["change"].get<json::boolean_t>()) {
                        auto txid = QString::fromStdString(i["txid"].get<json::string_t>());
                        txids.insert(txid);    

                        // Check for Memos
                        QString memoBytes = QString::fromStdString(i["memo"].get<json::string_t>());
                        if (!memoBytes.startsWith("f600"))  {
                            QString memo(QByteArray::fromHex(
                                            QByteArray::fromStdString(i["memo"].get<json::string_t>())));
                            if (!memo.trimmed().isEmpty())
                                memos[zaddr + txid] = memo;
                        }
                    }
                }        
            }

            // 2. For all txids, go and get the details of that txid.
            getBatchRPC<QString>(txids.toList(),
                [=] (QString txid) {
                    json payload = {
                        {"jsonrpc", "1.0"},
                        {"id",  "gettx"},
                        {"method", "gettransaction"},
                        {"params", {txid.toStdString()}}
                    };

                    return payload;
                },
                [=] (QMap<QString, json>* txidDetails) {
                    QList<TransactionItem> txdata;

                    // Combine them both together. For every zAddr's txid, get the amount, fee, confirmations and time
                    for (auto it = zaddrTxids->constBegin(); it != zaddrTxids->constEnd(); it++) {                        
                        for (auto& i : it.value().get<json::array_t>()) {   
                            // Filter out change txs
                            if (i["change"].get<json::boolean_t>())
                                continue;
                            
                            auto zaddr = it.key();
                            auto txid  = QString::fromStdString(i["txid"].get<json::string_t>());

                            // Lookup txid in the map
                            auto txidInfo = txidDetails->value(txid);

                            unsigned long timestamp;
                            if (txidInfo.find("time") != txidInfo.end()) {
                                timestamp = txidInfo["time"].get<json::number_unsigned_t>();
                            } else {
                                timestamp = txidInfo["blocktime"].get<json::number_unsigned_t>();
                            }
                            
                            auto amount        = i["amount"].get<json::number_float_t>();
                            auto confirmations = (unsigned long)txidInfo["confirmations"].get<json::number_unsigned_t>();                            

                            TransactionItem tx{ QString("receive"), timestamp, zaddr, txid, amount, 
                                                confirmations, "", memos.value(zaddr + txid, "") };
                            txdata.push_front(tx);
                        }
                    }

                    transactionsTableModel->addZRecvData(txdata);

                    // Cleanup both responses;
                    delete zaddrTxids;
                    delete txidDetails;
                }
            );
        }
    );
} 


/// This will refresh all the balance data from zcashd
void RPC::refresh(bool force) {
    getInfoThenRefresh(force);
}


void RPC::getInfoThenRefresh(bool force) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "getinfo"}
    };

    doRPC(payload, [=] (const json& reply) {   
		// Testnet?
		if (!reply["testnet"].is_null()) {
			Settings::getInstance()->setTestnet(reply["testnet"].get<json::boolean_t>());
		};

		// Connected, so display checkmark.
		QIcon i(":/icons/res/connected.png");
		main->statusIcon->setPixmap(i.pixmap(16, 16));

        static int    lastBlock = 0;
        int curBlock  = reply["blocks"].get<json::number_integer_t>();

        if ( force || (curBlock != lastBlock) ) {
            // Something changed, so refresh everything.
            lastBlock = curBlock;

            refreshBalances();		
            refreshAddresses(); // This calls refreshZSentTransactions() and refreshReceivedZTrans()
            refreshTransactions();
        }

		// Call to see if the blockchain is syncing. 
		json payload = {
			{"jsonrpc", "1.0"},
			{"id", "someid"},
			{"method", "getblockchaininfo"}
		};

		doRPC(payload, [=](const json& reply) {
            auto progress    = reply["verificationprogress"].get<double>();
            bool isSyncing   = progress < 0.999; // 99.9%
            int  blockNumber = reply["blocks"].get<json::number_unsigned_t>();

            Settings::getInstance()->setSyncing(isSyncing);
            Settings::getInstance()->setBlockNumber(blockNumber);

			QString statusText = QString() %
				(isSyncing ? "Syncing" : "Connected") %
				" (" %
				(Settings::getInstance()->isTestnet() ? "testnet:" : "") %
				QString::number(blockNumber) %
				(isSyncing ? ("/" % QString::number(progress*100, 'f', 0) % "%") : QString()) %
				")";
			main->statusLabel->setText(statusText);	
            auto zecPrice = Settings::getInstance()->getUSDFormat(1);
            if (!zecPrice.isEmpty()) {
                main->statusLabel->setToolTip("1 ZEC = " + zecPrice);
                main->statusIcon->setToolTip("1 ZEC = " + zecPrice);
            }
		});

    });        
}

void RPC::refreshAddresses() {
    delete zaddresses;
    zaddresses = new QList<QString>();

    getZAddresses([=] (json reply) {
        for (auto& it : reply.get<json::array_t>()) {   
            auto addr = QString::fromStdString(it.get<json::string_t>());
            zaddresses->push_back(addr);
        }

        // Refresh the sent and received txs from all these zaddresses
        refreshSentZTrans();
        refreshReceivedZTrans(*zaddresses);
    });
}

// Function to create the data model and update the views, used below.
void RPC::updateUI(bool anyUnconfirmed) {
    if (Settings::getInstance()->isTestnet()) {
        // See if the turnstile migration has any steps that need to be done.
        turnstile->executeMigrationStep();
    } else {
        // Not available on mainnet yet.
        main->ui->actionTurnstile_Migration->setVisible(false);
    }    

	ui->unconfirmedWarning->setVisible(anyUnconfirmed);

	// Update balances model data, which will update the table too
	balancesTableModel->setNewData(allBalances, utxos);

	// Add all the addresses into the inputs combo box
	auto lastFromAddr = ui->inputsCombo->currentText().split("(")[0].trimmed();

	ui->inputsCombo->clear();
	auto i = allBalances->constBegin();
	while (i != allBalances->constEnd()) {
		QString item = i.key() % "(" % QString::number(i.value(), 'g', 8) % " " % Utils::getTokenName() % ")";
		ui->inputsCombo->addItem(item);
		if (item.startsWith(lastFromAddr)) ui->inputsCombo->setCurrentText(item);

		++i;
	}
};

// Function to process reply of the listunspent and z_listunspent API calls, used below.
bool RPC::processUnspent(const json& reply) {
	bool anyUnconfirmed = false;
	for (auto& it : reply.get<json::array_t>()) {
		QString qsAddr = QString::fromStdString(it["address"]);
		auto confirmations = it["confirmations"].get<json::number_unsigned_t>();
		if (confirmations == 0) {
			anyUnconfirmed = true;
		}

		utxos->push_back(
			UnspentOutput{ qsAddr, QString::fromStdString(it["txid"]),
                            QString::number(it["amount"].get<json::number_float_t>(), 'g', 8),
                            (int)confirmations, it["spendable"].get<json::boolean_t>() });

		(*allBalances)[qsAddr] = (*allBalances)[qsAddr] + it["amount"].get<json::number_float_t>();
	}
	return anyUnconfirmed;
};

void RPC::refreshBalances() {    
    // 1. Get the Balances
    getBalance([=] (json reply) {    
        auto balT = QString::fromStdString(reply["transparent"]).toDouble();
        auto balZ = QString::fromStdString(reply["private"]).toDouble();
        auto tot  = QString::fromStdString(reply["total"]).toDouble();

        ui->balSheilded   ->setText(Settings::getInstance()->getZECDisplayFormat(balZ));
        ui->balTransparent->setText(Settings::getInstance()->getZECDisplayFormat(balT));
        ui->balTotal      ->setText(Settings::getInstance()->getZECDisplayFormat(tot));

        ui->balSheilded   ->setToolTip(Settings::getInstance()->getUSDFormat(balZ));
        ui->balTransparent->setToolTip(Settings::getInstance()->getUSDFormat(balT));
        ui->balTotal      ->setToolTip(Settings::getInstance()->getUSDFormat(tot));
    });

    // 2. Get the UTXOs
    // First, create a new UTXO list, deleting the old one;
    delete utxos;
    utxos = new QList<UnspentOutput>();
    delete allBalances;
    allBalances = new QMap<QString, double>();

    // Call the Transparent and Z unspent APIs serially and then, once they're done, update the UI
    getTransparentUnspent([=] (json reply) {
        auto anyTUnconfirmed = processUnspent(reply);

        getZUnspent([=] (json reply) {
            auto anyZUnconfirmed = processUnspent(reply);

            updateUI(anyTUnconfirmed || anyZUnconfirmed);    
        });        
    });
}

void RPC::refreshTransactions() {    
    getTransactions([=] (json reply) {
        QList<TransactionItem> txdata;

        for (auto& it : reply.get<json::array_t>()) {  
			double fee = 0;
			if (!it["fee"].is_null()) {
				fee = it["fee"].get<json::number_float_t>();
			}

            TransactionItem tx{
                QString::fromStdString(it["category"]),
                (unsigned long)it["time"].get<json::number_unsigned_t>(),
                (it["address"].is_null() ? "" : QString::fromStdString(it["address"])),
                QString::fromStdString(it["txid"]),
                it["amount"].get<json::number_float_t>() + fee,
                (unsigned long)it["confirmations"].get<json::number_unsigned_t>(),
                "", "" };

            txdata.push_back(tx);
        }

        // Update model data, which updates the table view
        transactionsTableModel->addTData(txdata);        
    });
}

// Read sent Z transactions from the file.
void RPC::refreshSentZTrans() {
    auto sentZTxs = SentTxStore::readSentTxFile();

    QList<QString> txids;

    for (auto sentTx: sentZTxs) {
        txids.push_back(sentTx.txid);
    }

    // Look up all the txids to get the confirmation count for them. 
    getBatchRPC<QString>(txids,
        [=] (QString txid) {
            json payload = {
                {"jsonrpc", "1.0"},
                {"id", "senttxid"},
                {"method", "gettransaction"},
                {"params", {txid.toStdString()}} 
            };

            return payload;
        },          
        [=] (QMap<QString, json>* txidList) {
            auto newSentZTxs = sentZTxs;
            // Update the original sent list with the confirmation count
            // TODO: This whole thing is kinda inefficient. We should probably just update the file
            // with the confirmed block number, so we don't have to keep calling gettransaction for the
            // sent items.
            for (TransactionItem& sentTx: newSentZTxs) {
                auto j = txidList->value(sentTx.txid);
                if (j.is_null())
                    continue;
                auto error = j["confirmations"].is_null();
                if (!error)
                    sentTx.confirmations = j["confirmations"].get<json::number_unsigned_t>();
            }
            
            transactionsTableModel->addZSentData(newSentZTxs);
        }
     );
}

void RPC::addNewTxToWatch(Tx tx, const QString& newOpid) {
    watchingOps.insert(newOpid, tx);

    watchTxStatus();
}

void RPC::watchTxStatus() {
    // Make an RPC to load pending operation statues
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_getoperationstatus"},
    };

    doRPC(payload, [=] (const json& reply) {
        // There's an array for each item in the status
        for (auto& it : reply.get<json::array_t>()) {  
            // If we were watching this Tx and it's status became "success", then we'll show a status bar alert
            QString id = QString::fromStdString(it["id"]);
            if (watchingOps.contains(id)) {
                // And if it ended up successful
                QString status = QString::fromStdString(it["status"]);
                if (status == "success") {
                    auto txid = QString::fromStdString(it["result"]["txid"]);
					
                    SentTxStore::addToSentTx(watchingOps.value(id), txid);

                    main->ui->statusBar->showMessage(Utils::txidStatusMessage + " " + txid);
                    main->loadingLabel->setVisible(false);

                    watchingOps.remove(id);

                    // Refresh balances to show unconfirmed balances                    
                    refresh(true);  
                } else if (status == "failed") {
                    // If it failed, then we'll actually show a warning. 
                    auto errorMsg = QString::fromStdString(it["error"]["message"]);
                    QMessageBox msg(
                        QMessageBox::Critical,
                        "Transaction Error", 
                        "The transaction with id " % id % " failed. The error was:\n\n" % errorMsg,
                        QMessageBox::Ok,
                        main
                    );
                    
                    watchingOps.remove(id);
                    
                    main->ui->statusBar->showMessage(" Tx " % id % " failed", 15 * 1000);
                    main->loadingLabel->setVisible(false);

                    msg.exec();                                                  
                } 
            }

            if (watchingOps.isEmpty()) {
                txTimer->start(Utils::updateSpeed);
            } else {
                txTimer->start(Utils::quickUpdateSpeed);
            }
        }

        // If there is some op that we are watching, then show the loading bar, otherwise hide it
        if (watchingOps.empty()) {
            main->loadingLabel->setVisible(false);
        } else {
            main->loadingLabel->setVisible(true);
            main->loadingLabel->setToolTip(QString::number(watchingOps.size()) + " tx computing. This can take several minutes.");
        }
    });
}

// Get the ZEC->USD price from coinmarketcap using their API
void RPC::refreshZECPrice() {
    QUrl cmcURL("https://api.coinmarketcap.com/v1/ticker/");

    QNetworkRequest req;
    req.setUrl(cmcURL);
    
    QNetworkReply *reply = restclient->get(req);

    QObject::connect(reply, &QNetworkReply::finished, [=] {
        reply->deleteLater();

        try {
            if (reply->error() != QNetworkReply::NoError) {
                auto parsed = json::parse(reply->readAll(), nullptr, false);
                if (!parsed.is_discarded() && !parsed["error"]["message"].is_null()) {
                    qDebug() << QString::fromStdString(parsed["error"]["message"]);    
                } else {
                    qDebug() << reply->errorString();
                }
                Settings::getInstance()->setZECPrice(0);
                return;
            } 

            auto all = reply->readAll();
            
            auto parsed = json::parse(all, nullptr, false);
            if (parsed.is_discarded()) {
                Settings::getInstance()->setZECPrice(0);
                return;
            }

            for (const json& item : parsed.get<json::array_t>()) {
                if (item["symbol"].get<json::string_t>().compare("ZEC") == 0) {
                    QString price = QString::fromStdString(item["price_usd"].get<json::string_t>());
                    qDebug() << "ZEC Price=" << price;
                    Settings::getInstance()->setZECPrice(price.toDouble());

                    return;
                }
            }
        } catch (...) {
            // If anything at all goes wrong, just set the price to 0 and move on.
            qDebug() << QString("Caught something nasty");
        }

        // If nothing, then set the price to 0;
        Settings::getInstance()->setZECPrice(0);
    });
}
