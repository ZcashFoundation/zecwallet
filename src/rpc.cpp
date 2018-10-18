#include "rpc.h"
#include "utils.h"
#include "transactionitem.h"
#include "settings.h"

using json = nlohmann::json;

RPC::RPC(QNetworkAccessManager* client, MainWindow* main) {
	this->restclient = client;
	this->main = main;
	this->ui = main->ui;

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

	// Set up a timer to refresh the UI every few seconds
	timer = new QTimer(main);
	QObject::connect(timer, &QTimer::timeout, [=]() {
		refresh();
	});
	timer->start(Utils::updateSpeed);    

    // Set up the timer to watch for tx status
    txTimer = new QTimer(main);
    QObject::connect(txTimer, &QTimer::timeout, [=]() {
        refreshTxStatus();
    });
    // Start at every 10s. When an operation is pending, this will change to every second
    txTimer->start(Utils::updateSpeed);  
}

RPC::~RPC() {
    delete timer;
    delete txTimer;

    delete transactionsTableModel;
    delete balancesTableModel;

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

void RPC::newZaddr(const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_getnewaddress"},
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

void RPC::doSendRPC(const json& payload, const std::function<void(json)>& cb) {
    QNetworkReply *reply = restclient->post(request, QByteArray::fromStdString(payload.dump()));

    QObject::connect(reply, &QNetworkReply::finished, [=] {
        reply->deleteLater();
        
        if (reply->error() != QNetworkReply::NoError) {
            auto parsed = json::parse(reply->readAll(), nullptr, false);
            if (!parsed.is_discarded() && !parsed["error"]["message"].is_null()) {
                handleTxError(QString::fromStdString(parsed["error"]["message"]));    
            } else {
                handleTxError(reply->errorString());
            }
            
            return;
        } 
        
        auto parsed = json::parse(reply->readAll(), nullptr, false);
        if (parsed.is_discarded()) {
            handleTxError("Unknown error");
        }
        
        cb(parsed["result"]);
    });
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
        } else if (error.contains("connection", Qt::CaseInsensitive)) {
            explanation = QString()
                        % "\n\nThis is most likely because we couldn't connect to zcashd. Is zcashd running and " 
                        % "accepting connections from this machine? \nIf you need to change the host/port, you can set that in the "
                        % "File->Settings menu.";
        } else if (error.contains("bad request", Qt::CaseInsensitive)) {
            explanation = QString()
                        % "\n\nThis is most likely an internal error. Are you using zcashd v2.0 or higher? You might "
                        % "need to file a bug report here: https://github.com/adityapk00/zcash-qt-wallet/issues";
        } else if (error.contains("internal server error", Qt::CaseInsensitive) ||
                   error.contains("rewinding", Qt::CaseInsensitive) || 
                   error.contains("loading", Qt::CaseInsensitive)) {
            explanation = QString()
                        % "\n\nIf you just started zcashd, then it's still loading and you might have to wait a while. If zcashd is ready, then you've run into  "
                        % "something unexpected, and might need to file a bug report here: https://github.com/adityapk00/zcash-qt-wallet/issues";
        } else {
            explanation = QString()
                        % "\n\nThis is most likely an internal error. Something unexpected happened. "
                        % "You might need to file a bug report here: https://github.com/adityapk00/zcash-qt-wallet/issues";
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


/// This will refresh all the balance data from zcashd
void RPC::refresh() {
    // First, test the connection to see if we can actually get info.
    getInfoThenRefresh();
}


void RPC::getInfoThenRefresh() {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "getinfo"}
    };

    doRPC(payload, [=] (const json& reply) {   
		// Testnet?
		if (reply.find("testnet") != reply.end()) {
			Settings::getInstance()->setTestnet(reply["testnet"].get<json::boolean_t>());
		};

		// Connected, so display checkmark.
		QIcon i(":/icons/res/connected.png");
		main->statusIcon->setPixmap(i.pixmap(16, 16));

		// Refresh everything.
		refreshBalances();
		refreshTransactions();
		refreshAddresses();

		// Call to see if the blockchain is syncing. 
		json payload = {
			{"jsonrpc", "1.0"},
			{"id", "someid"},
			{"method", "getblockchaininfo"}
		};

		doRPC(payload, [=](const json& reply) {
			double progress = reply["verificationprogress"].get<double>();
			QString statusText = QString() %
				(progress < 0.99 ? "Syncing" : "Connected") %
				" (" %
				(Settings::getInstance()->isTestnet() ? "testnet:" : "") %
				QString::number(reply["blocks"].get<json::number_unsigned_t>()) %
				(progress < 0.99 ? ("/" % QString::number(progress*100, 'f', 0) % "%") : QString()) %
				")";
			main->statusLabel->setText(statusText);			
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
    });
}

void RPC::refreshBalances() {    
    // 1. Get the Balances
    getBalance([=] (json reply) {        
        ui->balSheilded     ->setText(QString::fromStdString(reply["private"]) % " " % Utils::getTokenName());
        ui->balTransparent  ->setText(QString::fromStdString(reply["transparent"]) % " " % Utils::getTokenName());
        ui->balTotal        ->setText(QString::fromStdString(reply["total"]) % " " % Utils::getTokenName());
    });

    // 2. Get the UTXOs
    // First, create a new UTXO list, deleting the old one;
    delete utxos;
    utxos = new QList<UnspentOutput>();
    delete allBalances;
    allBalances = new QMap<QString, double>();

    // Function to process reply of the listunspent and z_listunspent API calls, used below.
    auto processUnspent = [=] (const json& reply) -> bool { 
        bool anyUnconfirmed = false;
        for (auto& it : reply.get<json::array_t>()) {   
            QString qsAddr      = QString::fromStdString(it["address"]);
            auto confirmations  = it["confirmations"].get<json::number_unsigned_t>();
            if (confirmations == 0) {
                anyUnconfirmed = true;
            }

            utxos->push_back(
                UnspentOutput(
                    qsAddr,
                    QString::fromStdString(it["txid"]), 
                    QString::number(it["amount"].get<json::number_float_t>(), 'g', 8),
                    confirmations
                )
            );

            (*allBalances)[qsAddr] = (*allBalances)[qsAddr] + it["amount"].get<json::number_float_t>();
        }
        return anyUnconfirmed;
    };

    // Function to create the data model and update the views, used below.
    auto updateUI = [=] (bool anyUnconfirmed) {       
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
    auto txdata = new QList<TransactionItem>();
    /*
    auto getZReceivedTransactions = ([=] (const json& reply) {
        for (auto& it : reply.get<json::array_t>()) {  
            TransactionItem tx(
                QString("receive"),
                QDateTime::fromSecsSinceEpoch(it["time"].get<json::number_unsigned_t>()).toLocalTime().toString(),
                (it["address"].is_null() ? "" : QString::fromStdString(it["address"])),
                QString::fromStdString(it["txid"]),
                it["amount"].get<json::number_float_t>(),
                it["confirmations"].get<json::number_float_t>()
            );

            txdata->push_front(tx);
        }
    });
    */

    getTransactions([=] (json reply) {
        for (auto& it : reply.get<json::array_t>()) {  
            TransactionItem tx(
                QString::fromStdString(it["category"]),
                QDateTime::fromSecsSinceEpoch(it["time"].get<json::number_unsigned_t>()).toLocalTime().toString(),
                (it["address"].is_null() ? "" : QString::fromStdString(it["address"])),
                QString::fromStdString(it["txid"]),
                it["amount"].get<json::number_float_t>(),
                it["confirmations"].get<json::number_float_t>()
            );

            txdata->push_front(tx);
        }

        // Update model data, which updates the table view
        transactionsTableModel->setNewData(txdata);        
    });
}

void RPC::refreshTxStatus(const QString& newOpid) {
    if (!newOpid.isEmpty()) {
        watchingOps.insert(newOpid);
    }

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
					qDebug() << "Tx completed: " << txid;
                    main->ui->statusBar->showMessage(Utils::txidStatusMessage + " " + txid);
                    main->loadingLabel->setVisible(false);

                    watchingOps.remove(id);
                    txTimer->start(Utils::updateSpeed);

                    // Refresh balances to show unconfirmed balances                    
                    refresh();  
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
                    txTimer->start(10 * 1000);                    
                    
                    main->ui->statusBar->showMessage(" Tx " % id % " failed", 15 * 1000);
                    main->loadingLabel->setVisible(false);

                    msg.exec();                                                  
                } else if (status == "executing") {
                    // If the operation is executing, then watch every second. 
                    txTimer->start(1 * 1000);
                }
            }
        }

        // If there is some op that we are watching, then show the loading bar, otherwise hide it
        if (watchingOps.size() == 0) {
            main->loadingLabel->setVisible(false);
        } else {
            main->loadingLabel->setVisible(true);
            main->loadingLabel->setToolTip(QString::number(watchingOps.size()) + " tx computing. This can take several minutes.");
        }
    });
}
