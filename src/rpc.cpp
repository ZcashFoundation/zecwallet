#include "rpc.h"
#include "transactionitem.h"
#include "settings.h"

using json = nlohmann::json;

RPC::RPC(QNetworkAccessManager* client, MainWindow* main) {
	this->restclient = client;
	this->main = main;
	this->ui = main->ui;

	reloadConnectionInfo();

	// Set up a timer to refresh the UI every few seconds
	timer = new QTimer(main);
	QObject::connect(timer, &QTimer::timeout, [=]() {
		refresh();
	});
	timer->start(1 * 60 * 1000);    // Query every minute

    // Set up the timer to watch for tx status
    txTimer = new QTimer(main);
    QObject::connect(txTimer, &QTimer::timeout, [=]() {
        refreshTxStatus();
    });
    // Start at every minute. When an operation is pending, this will change to every second
    txTimer->start(1 * 60 * 1000);  
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
    myurl.setHost(main->getSettings()->getHost());
    myurl.setPort(main->getSettings()->getPort().toInt());

    request.setUrl(myurl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    
    QString headerData = "Basic " + main->getSettings()->getUsernamePassword().toLocal8Bit().toBase64();
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
                        % "need to file a bug report here: ";
        } else if (error.contains("internal server error", Qt::CaseInsensitive)) {
            explanation = QString()
                        % "\n\nIf you just started zcashd, then it's still loading and you might have to wait a while. If zcashd is ready, then you've run into  "
                        % "something unexpected, and might need to file a bug report here: ";
        } else {
            explanation = QString()
                        % "\n\nThis is most likely an internal error. Something unexpected happened. "
                        % "You might need to file a bug report here: ";
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

    doRPC(payload, [=] (json reply) {        
        QString statusText = QString::fromStdString("Connected (")
                                .append(QString::number(reply["blocks"].get<json::number_unsigned_t>()))
                                .append(")");
        main->statusLabel->setText(statusText);
        main->statusIcon->clear();  // TODO: Add checked icon

        // Refresh everything.
        refreshBalances();
        refreshTransactions();
        refreshAddresses();
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
    ui->unconfirmedWarning->setVisible(false);        
    
    // 1. Get the Balances
    getBalance([=] (json reply) {        
        ui->balSheilded     ->setText(QString::fromStdString(reply["private"]) % " ZEC");
        ui->balTransparent  ->setText(QString::fromStdString(reply["transparent"]) % " ZEC");
        ui->balTotal        ->setText(QString::fromStdString(reply["total"]) % " ZEC");
    });

    // 2. Get the UTXOs
    // First, create a new UTXO list, deleting the old one;
    delete utxos;
    utxos = new QList<UnspentOutput>();
    delete allBalances;
    allBalances = new QMap<QString, double>();

    // Function to process reply of the listunspent and z_listunspent API calls, used below.
    auto processUnspent = [=] (const json& reply) { 
        for (auto& it : reply.get<json::array_t>()) {   
            QString qsAddr      = QString::fromStdString(it["address"]);
            auto confirmations  = it["confirmations"].get<json::number_unsigned_t>();
            if (confirmations == 0) {
                ui->unconfirmedWarning->setVisible(true);
            }

            utxos->push_back(
                UnspentOutput(
                    qsAddr,
                    QString::fromStdString(it["txid"]), 
                    QString::number(it["amount"].get<json::number_float_t>(), 'f', 8),
                    confirmations
                )
            );

            (*allBalances)[qsAddr] = (*allBalances)[qsAddr] + it["amount"].get<json::number_float_t>();
        }
    };

    // Function to create the data model and update the views, used below.
    auto updateUI = [=] () {
        // Create the balances table model.
        
        // Delete the old Model, because the setModel() doesn't take ownership of the Model object
        delete balancesTableModel;

        balancesTableModel = new BalancesTableModel(ui->balancesTable, allBalances, utxos);
        ui->balancesTable->setModel(balancesTableModel);

        // Configure Balances Table
        ui->balancesTable->setColumnWidth(0, 300);

        // Add all the addresses into the inputs combo box
        auto lastFromAddr = ui->inputsCombo->currentText().split("(")[0].trimmed();

        ui->inputsCombo->clear();
        auto i = allBalances->constBegin();
        while (i != allBalances->constEnd()) {
            QString item = i.key() % "(" % QString::number(i.value(), 'f', 8) % " ZEC)";
            ui->inputsCombo->addItem(item);
            if (item.startsWith(lastFromAddr)) ui->inputsCombo->setCurrentText(item);

            ++i;
        }        
    };

    // Call the Transparent and Z unspent APIs serially and then, once they're done, update the UI
    getTransparentUnspent([=] (json reply) {
        processUnspent(reply);

        getZUnspent([=] (json reply) {
            processUnspent(reply);

            updateUI();    
        });        
    });
}

void RPC::refreshTransactions() {
    // First, delete the previous headers
    delete transactionsTableModel;
    
    auto txdata = new QList<TransactionItem>();

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

        transactionsTableModel = new TxTableModel(ui->transactionsTable, txdata);
        ui->transactionsTable->setModel(transactionsTableModel);

        ui->transactionsTable->setColumnWidth(1, 300);
        ui->transactionsTable->setColumnWidth(2, 200);
        ui->transactionsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    });
}

void RPC::refreshTxStatus(const QString& newOpid) {
    if (!newOpid.isEmpty()) {
        qDebug() << QString::fromStdString("Adding opid ") % newOpid;
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
                qDebug() << QString::fromStdString("Watching opid, current status= ") % status;
                if (status == "success") {
                    qDebug() << QString::fromStdString("Success, showing status message");
                    main->ui->statusBar->showMessage(" Tx " % id % " computed successfully and submitted");
                    
                    watchingOps.remove(id);
                    txTimer->start(1 * 60 * 1000);

                    // Refresh balances to show unconfirmed balances
                    refresh();  
                } else if (status == "failed") {
                    // If it failed, then we'll actually show a warning. 
                    auto errorMsg = QString::fromStdString(it["error"]["message"]);
                    qDebug() << QString::fromStdString("Failed with message") % errorMsg;
                    QMessageBox msg(
                        QMessageBox::Critical,
                        "Transaction Error", 
                        "The transaction with id " % id % " failed. The error was:\n\n" % errorMsg,
                        QMessageBox::Ok,
                        main
                    );
                    
                    txTimer->start(1 * 60 * 1000);
                    watchingOps.remove(id);     
                    main->ui->statusBar->showMessage(" Tx " % id % " failed", 15 * 1000);

                    msg.exec();                                                  
                } else if (status == "executing") {
                    // If the operation is executing, then watch every second. 
                    qDebug() << QString::fromStdString("executing, doing again in 1 sec");
                    txTimer->start(1 * 1000);
                }
            }
        }
    });
}


    /*
    [
    {
        "id": "opid-ad54b34c-1d89-48af-9cb5-4825905fc62e",
        "status": "executing",
        "creation_time": 1539490482,
        "method": "z_sendmany",
        "params": {
        "fromaddress": "t1aWhRh9JNKWzpzjn2gmULDJzfKLC724EPS",
        "amounts": [
            {
            "address": "zcVHg9euUSQF8ef7ZXztrv4LcdC1mytEUYLuoj4W5iSygLFYjm1yQCggAYnnydUaHLy2GBaxF4PX5vjaJjnj2Lq3ecQtGF4",
            "amount": 0.0001
            }
        ],
        "minconf": 1,
        "fee": 0.0001
        }
    },{
        "id": "opid-7807e672-7f8c-428a-8587-3354d0ae1b88",
        "status": "failed",
        "creation_time": 1539490847,
        "error": {
            "code": -6,
            "message": "Insufficient protected funds, have 0.0006001, need 111.0001"
        },
    ]
    */