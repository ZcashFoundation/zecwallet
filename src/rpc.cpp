#include "rpc.h"

#include "addressbook.h"
#include "settings.h"
#include "senttxstore.h"
#include "turnstile.h"
#include "version.h"
#include "websockets.h"

using json = nlohmann::json;

RPC::RPC(MainWindow* main) {
    auto cl = new ConnectionLoader(main, this);

    // Execute the load connection async, so we can set up the rest of RPC properly. 
    QTimer::singleShot(1, [=]() { cl->loadConnection(); });

    this->main = main;
    this->ui = main->ui;

    this->turnstile = new Turnstile(this, main);

    // Setup balances table model
    balancesTableModel = new BalancesTableModel(main->ui->balancesTable);
    main->ui->balancesTable->setModel(balancesTableModel);

    // Setup transactions table model
    transactionsTableModel = new TxTableModel(ui->transactionsTable);
    main->ui->transactionsTable->setModel(transactionsTableModel);
    
    // Set up timer to refresh Price
    priceTimer = new QTimer(main);
    QObject::connect(priceTimer, &QTimer::timeout, [=]() {
        if (Settings::getInstance()->getAllowFetchPrices())
            refreshZECPrice();
    });
    priceTimer->start(Settings::priceRefreshSpeed);  // Every hour

    // Set up a timer to refresh the UI every few seconds
    timer = new QTimer(main);
    QObject::connect(timer, &QTimer::timeout, [=]() {
        refresh();
    });
    timer->start(Settings::updateSpeed);    

    // Set up the timer to watch for tx status
    txTimer = new QTimer(main);
    QObject::connect(txTimer, &QTimer::timeout, [=]() {
        watchTxStatus();
    });
    // Start at every 10s. When an operation is pending, this will change to every second
    txTimer->start(Settings::updateSpeed);  

    usedAddresses = new QMap<QString, bool>();

    // Initialize the migration status to unavailable.
    this->migrationStatus.available = false;
}

RPC::~RPC() {
    delete timer;
    delete txTimer;

    delete transactionsTableModel;
    delete balancesTableModel;
    delete turnstile;

    delete utxos;
    delete allBalances;
    delete usedAddresses;
    delete zaddresses;
    delete taddresses;

    delete conn;
}

void RPC::setEZcashd(QProcess* p) {
    ezcashd = p;

    if ((ezcashd && ui->tabWidget->widget(4) == nullptr) && (ezcashd && ui->tabWidget->widget(5) == nullptr)) {
		ui->tabWidget->addTab(main->safenodestab, "SafeNodes") && ui->tabWidget->addTab(main->zcashdtab, "safecoind");
    }
}

// Called when a connection to safecoind is available. 
void RPC::setConnection(Connection* c) {
    if (c == nullptr) return;

    delete conn;
    this->conn = c;

    ui->statusBar->showMessage("Ready! Thank you for helping secure the Safecoin network by running a full node.");

    // See if we need to remove the reindex/rescan flags from the safecoin.conf file
    auto zcashConfLocation = Settings::getInstance()->getZcashdConfLocation();
    Settings::removeFromZcashConf(zcashConfLocation, "rescan");
    Settings::removeFromZcashConf(zcashConfLocation, "reindex");

    // If we're allowed to get the Zec Price, get the prices
    if (Settings::getInstance()->getAllowFetchPrices())
        refreshZECPrice();

    // If we're allowed to check for updates, check for a new release
    if (Settings::getInstance()->getCheckForUpdates())
        checkForUpdate();

    // Force update, because this might be coming from a settings update
    // where we need to immediately refresh
    refresh(true);
}

void RPC::getTAddresses(const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "getaddressesbyaccount"},
        {"params", {""}}
    };

    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void RPC::getZAddresses(const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_listaddresses"},
    };

    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void RPC::getTransparentUnspent(const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "listunspent"},
        {"params", {0}}             // Get UTXOs with 0 confirmations as well.
    };

    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void RPC::getZUnspent(const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_listunspent"},
        {"params", {0}}             // Get UTXOs with 0 confirmations as well.
    };

    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void RPC::newZaddr(bool sapling, const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_getnewaddress"},
        {"params", { sapling ? "sapling" : "sprout" }},
    };
    
    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void RPC::newTaddr(const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "getnewaddress"},
    };

    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void RPC::getZPrivKey(QString addr, const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_exportkey"},
        {"params", { addr.toStdString() }},
    };
    
    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void RPC::getTPrivKey(QString addr, const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "dumpprivkey"},
        {"params", { addr.toStdString() }},
    };
    
    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void RPC::importZPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_importkey"},
        {"params", { addr.toStdString(), (rescan? "yes" : "no") }},
    };
    
    conn->doRPCWithDefaultErrorHandling(payload, cb);
}


void RPC::importTPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "importprivkey"},
        {"params", { addr.toStdString(), (rescan? "yes" : "no") }},
    };
    
    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void RPC::validateAddress(QString address, const std::function<void(json)>& cb) {
    QString method = Settings::isZAddress(address) ? "z_validateaddress" : "validateaddress";

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", method.toStdString() },
        {"params", { address.toStdString() } },
    };
    
    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void RPC::getBalance(const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_gettotalbalance"},
        {"params", {0}}             // Get Unconfirmed balance as well.
    };

    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void RPC::getTransactions(const std::function<void(json)>& cb) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "listtransactions"}
    };

    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void RPC::sendZTransaction(json params, const std::function<void(json)>& cb, 
    const std::function<void(QString)>& err) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_sendmany"},
        {"params", params}
    };

    conn->doRPC(payload, cb,  [=] (auto reply, auto parsed) {
        if (!parsed.is_discarded() && !parsed["error"]["message"].is_null()) {
            err(QString::fromStdString(parsed["error"]["message"]));    
        } else {
            err(reply->errorString());
        }
    });
}

/**
 * Method to get all the private keys for both z and t addresses. It will make 2 batch calls,
 * combine the result, and call the callback with a single list containing both the t-addr and z-addr
 * private keys
 */ 
void RPC::getAllPrivKeys(const std::function<void(QList<QPair<QString, QString>>)> cb) {
    if (conn == nullptr) {
        // No connection, just return
        return;
    }

    // A special function that will call the callback when two lists have been added
    auto holder = new QPair<int, QList<QPair<QString, QString>>>();
    holder->first = 0;  // This is the number of times the callback has been called, initialized to 0
    auto fnCombineTwoLists = [=] (QList<QPair<QString, QString>> list) {
        // Increment the callback counter
        holder->first++;    

        // Add all
        std::copy(list.begin(), list.end(), std::back_inserter(holder->second));
        
        // And if the caller has been called twice, do the parent callback with the 
        // collected list
        if (holder->first == 2) {
            // Sort so z addresses are on top
            std::sort(holder->second.begin(), holder->second.end(), 
                        [=] (auto a, auto b) { return a.first > b.first; });

            cb(holder->second);
            delete holder;
        }            
    };

    // A utility fn to do the batch calling
    auto fnDoBatchGetPrivKeys = [=](json getAddressPayload, std::string privKeyDumpMethodName) {
        conn->doRPCWithDefaultErrorHandling(getAddressPayload, [=] (json resp) {
            QList<QString> addrs;
            for (auto addr : resp.get<json::array_t>()) {   
                addrs.push_back(QString::fromStdString(addr.get<json::string_t>()));
            }

            // Then, do a batch request to get all the private keys
            conn->doBatchRPC<QString>(
                addrs, 
                [=] (auto addr) {
                    json payload = {
                        {"jsonrpc", "1.0"},
                        {"id", "someid"},
                        {"method", privKeyDumpMethodName},
                        {"params", { addr.toStdString() }},
                    };
                    return payload;
                },
                [=] (QMap<QString, json>* privkeys) {
                    QList<QPair<QString, QString>> allTKeys;
                    for (QString addr: privkeys->keys()) {
                        allTKeys.push_back(
                            QPair<QString, QString>(
                                addr, 
                                QString::fromStdString(privkeys->value(addr).get<json::string_t>())));
                    }

                    fnCombineTwoLists(allTKeys);
                    delete privkeys;
                }
            );
        });
    };

    // First get all the t and z addresses.
    json payloadT = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "getaddressesbyaccount"},
        {"params", {""} }
    };

    json payloadZ = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_listaddresses"}
    };

    fnDoBatchGetPrivKeys(payloadT, "dumpprivkey");
    fnDoBatchGetPrivKeys(payloadZ, "z_exportkey");
}


// Build the RPC JSON Parameters for this tx
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
        // Force it through string for rounding. Without this, decimal points beyond 8 places
        // will appear, causing an "invalid amount" error
        rec["amount"]       = Settings::getDecimalString(toAddr.amount).toStdString(); //.toDouble(); 
        if (Settings::isZAddress(toAddr.addr) && !toAddr.encodedMemo.trimmed().isEmpty())
            rec["memo"]     = toAddr.encodedMemo.toStdString();

        allRecepients.push_back(rec);
    }

    // Add sender    
    params.push_back(tx.fromAddr.toStdString());
    params.push_back(allRecepients);

    // Add fees if custom fees are allowed.
    if (Settings::getInstance()->getAllowCustomFees()) {
        params.push_back(1); // minconf
        params.push_back(tx.fee);
    }
}


void RPC::noConnection() {    
    QIcon i = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
    main->statusIcon->setPixmap(i.pixmap(16, 16));
    main->statusIcon->setToolTip("");
    main->statusLabel->setText(QObject::tr("No Connection"));
    main->statusLabel->setToolTip("");
    main->ui->statusBar->showMessage(QObject::tr("No Connection"), 1000);

    // Clear balances table.
    QMap<QString, double> emptyBalances;
    QList<UnspentOutput>  emptyOutputs;
    balancesTableModel->setNewData(&emptyBalances, &emptyOutputs);

    // Clear Transactions table.
    QList<TransactionItem> emptyTxs;
    transactionsTableModel->addTData(emptyTxs);
    transactionsTableModel->addZRecvData(emptyTxs);
    transactionsTableModel->addZSentData(emptyTxs);

    // Clear balances
    ui->balSheilded->setText("");
    ui->balTransparent->setText("");
    ui->balTotal->setText("");
    ui->balUSDTotal->setText("");

    ui->balSheilded->setToolTip("");
    ui->balTransparent->setToolTip("");
    ui->balTotal->setToolTip("");
    ui->balUSDTotal->setToolTip("");

    // Clear send tab from address
    ui->inputsCombo->clear();
}

// Refresh received z txs by calling z_listreceivedbyaddress/gettransaction
void RPC::refreshReceivedZTrans(QList<QString> zaddrs) {
    if  (conn == nullptr) 
        return noConnection();

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
    conn->doBatchRPC<QString>(zaddrs,
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
                    // Mark the address as used
                    usedAddresses->insert(zaddr, true);

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
            conn->doBatchRPC<QString>(txids.toList(),
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

                            qint64 timestamp;
                            if (txidInfo.find("time") != txidInfo.end()) {
                                timestamp = txidInfo["time"].get<json::number_unsigned_t>();
                            } else {
                                timestamp = txidInfo["blocktime"].get<json::number_unsigned_t>();
                            }
                            
                            auto amount        = i["amount"].get<json::number_float_t>();
                            auto confirmations = static_cast<long>(txidInfo["confirmations"].get<json::number_integer_t>());

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

/// This will refresh all the balance data from safecoind
void RPC::refresh(bool force) {
    if  (conn == nullptr) 
        return noConnection();

    getInfoThenRefresh(force);
}


void RPC::getInfoThenRefresh(bool force) {
    if  (conn == nullptr) 
        return noConnection();

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "getinfo"}
    };

    static bool prevCallSucceeded = false;
    conn->doRPC(payload, [=] (const json& reply) {   
        prevCallSucceeded = true;
        // Testnet?
        if (!reply["testnet"].is_null()) {
            Settings::getInstance()->setTestnet(reply["testnet"].get<json::boolean_t>());
        };

        // Recurring pamynets are testnet only
        if (!Settings::getInstance()->isTestnet())
            main->disableRecurring();

        // Connected, so display checkmark.
        QIcon i(":/icons/res/connected.gif");
        main->statusIcon->setPixmap(i.pixmap(16, 16));

        static int lastBlock    = 0;
        int curBlock            = reply["blocks"].get<json::number_integer_t>();
        int longestchain        = reply["longestchain"].get<json::number_integer_t>();
        int version             = reply["version"].get<json::number_integer_t>();
        int notarized           = reply["notarized"].get<json::number_integer_t>();
        int p2pport             = reply["p2pport"].get<json::number_integer_t>();
        int rpcport             = reply["rpcport"].get<json::number_integer_t>();
        int protocolversion     = reply["protocolversion"].get<json::number_integer_t>();
        int tls_connections     = reply["tls_connections"].get<json::number_integer_t>();
        QString safever          = QString::fromStdString( reply["SAFEversion"].get<json::string_t>() );
        QString ntzhash         = QString::fromStdString( reply["notarizedhash"].get<json::string_t>() );
        QString ntztxid         = QString::fromStdString( reply["notarizedtxid"].get<json::string_t>() );

        Settings::getInstance()->setZcashdVersion(version);

        ui->notarized->setText(QString::number(notarized));
        ui->longestchain->setText(QString::number(longestchain));
        ui->notarizedhashvalue->setText( ntzhash );
        ui->notarizedtxidvalue->setText( ntztxid );
        ui->version->setText( QString::number(version) );
        ui->safeversion->setText( safever );
        ui->protocolversion->setText( QString::number(protocolversion) );
        ui->tls_connections->setText( QString::number(tls_connections) );
        ui->p2pport->setText( QString::number(p2pport) );
        ui->rpcport->setText( QString::number(rpcport) );


        // See if recurring payments needs anything
        Recurring::getInstance()->processPending(main);

        if ( force || (curBlock != lastBlock) ) {
            // Something changed, so refresh everything.
            lastBlock = curBlock;

            // See if the turnstile migration has any steps that need to be done.
            turnstile->executeMigrationStep();

            refreshBalances();        
            refreshAddresses();     // This calls refreshZSentTransactions() and refreshReceivedZTrans()
            refreshTransactions();
            refreshMigration();     // Sapling turnstile migration status.
        }

        int connections = reply["connections"].get<json::number_integer_t>();
        Settings::getInstance()->setPeers(connections);

        if (connections == 0) {
            // If there are no peers connected, then the internet is probably off or something else is wrong. 
            QIcon i = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
            main->statusIcon->setPixmap(i.pixmap(16, 16));
        }

        // Get network sol/s
        json payload = {
            {"jsonrpc", "1.0"},
            {"id", "someid"},
            {"method", "getnetworksolps"}
        };

            conn->doRPCIgnoreError(payload, [=](const json& reply) {
                qint64 solrate = reply.get<json::number_unsigned_t>();

                ui->numconnections->setText(QString::number(connections));
                ui->solrate->setText(QString::number(solrate) % " Sol/s");
            });

        // Get activenodes
        payload = {
            {"jsonrpc", "1.0"},
            {"id", "someid"},
            {"method", "getactivenodes"}
        };
        conn->doRPCIgnoreError(payload, [=] (const json& reply) {
            double collateral_total;
            int node_count          = reply["node_count"].get<json::number_integer_t>();
            int tier_0_count        = reply["tier_0_count"].get<json::number_integer_t>();
            int tier_1_count        = reply["tier_1_count"].get<json::number_integer_t>();
            int tier_2_count        = reply["tier_2_count"].get<json::number_integer_t>();
            int tier_3_count        = reply["tier_3_count"].get<json::number_integer_t>();
            collateral_total    = reply["collateral_total"].get<json::number_float_t>();

            ui->node_count->setText(QString::number(node_count));
			
		if (!getConnection()->config->addrindex.isEmpty()) {
			
            ui->tier_0_count->setText(QString::number(tier_0_count));
            ui->tier_1_count->setText(QString::number(tier_1_count));
            ui->tier_2_count->setText(QString::number(tier_2_count));
            ui->tier_3_count->setText(QString::number(tier_3_count));
            ui->collateral_total->setToolTip(Settings::getZECDisplayFormat(collateral_total));
            ui->collateral_total->setText(Settings::getZECDisplayFormat(collateral_total));
            ui->collateral_total_usd->setToolTip(Settings::getUSDFromZecAmount(collateral_total));
            ui->collateral_total_usd->setText(Settings::getUSDFromZecAmount(collateral_total));

		} else {
				ui->tier_0_count->setText("addressindex not enabled");
				ui->tier_1_count->setText("addressindex not enabled");
				ui->tier_2_count->setText("addressindex not enabled");
				ui->tier_3_count->setText("addressindex not enabled");
				ui->collateral_total->setText("addressindex not enabled");
				ui->collateral_total_usd->setText("addressindex not enabled");
		}

        });


        // Get nodeinfo
        payload = {
            {"jsonrpc", "1.0"},
            {"id", "someid"},
            {"method", "getnodeinfo"}
        };
        conn->doRPCIgnoreError(payload, [=] (const json& reply) {
		
		double balance, collateral;
		int tier;
		int last_reg_height;
		int valid_thru_height;
		bool is_valid;

	if (!getConnection()->config->safenode.isEmpty()) {
		if (!getConnection()->config->addrindex.isEmpty()) {
			try
			{
				balance = reply["balance"].get<json::number_float_t>();
				
				ui->balance->setToolTip(Settings::getZECDisplayFormat(balance));
				ui->balance->setText(Settings::getZECDisplayFormat(balance));
				ui->balance_usd->setToolTip(Settings::getUSDFromZecAmount(balance));
				ui->balance_usd->setText(Settings::getUSDFromZecAmount(balance));
			}
			catch (...)
			{
				ui->balance->setText("unknown");
				ui->balance_usd->setText("unknown");
			}
			try
			{
				collateral = reply["collateral"].get<json::number_float_t>();
				
				ui->collateral->setToolTip(Settings::getZECDisplayFormat(collateral));
				ui->collateral->setText(Settings::getZECDisplayFormat(collateral));
				ui->collateral_usd->setToolTip(Settings::getUSDFromZecAmount(collateral));
				ui->collateral_usd->setText(Settings::getUSDFromZecAmount(collateral));
			}
			catch (...)
			{
				ui->collateral->setText("unknown");
				ui->collateral_usd->setText("unknown");
			}
		
			
			try
			{
				tier = reply["tier"].get<json::number_integer_t>();
				
				ui->tier->setText(QString::number(tier));
			}
			catch (...)
			{
				ui->tier->setText("unknown");
			}
		} else {
				ui->balance->setText("addressindex not enabled");
				ui->balance_usd->setText("addressindex not enabled");
				ui->collateral->setText("addressindex not enabled");
				ui->collateral_usd->setText("addressindex not enabled");
				ui->tier->setText("addressindex not enabled");
		}

			is_valid = reply["is_valid"].get<json::boolean_t>();
			
			std::vector<std::string> vs_errors = reply["errors"].get<std::vector<std::string>>();
			QString error_line;
			
			for (unsigned int i = 0; i < vs_errors.size(); i++)
			
			{
				error_line = error_line + QString(vs_errors.at(i).c_str()) + "\n";
			}
			
			ui->is_valid->setText(is_valid?"YES":"NO");
			ui->errors->setText(error_line);


		if (is_valid == true) {
			try
			{
				last_reg_height = reply["last_reg_height"].get<json::number_integer_t>();
				
				ui->last_reg_height->setText(QString::number(last_reg_height));
			}
			catch (...)
			{
				ui->last_reg_height->setText("unknown");
			}
			try
			{
				valid_thru_height = reply["valid_thru_height"].get<json::number_integer_t>();
				
				ui->valid_thru_height->setText(QString::number(valid_thru_height));
			}
			catch (...)
			{
				ui->valid_thru_height->setText("unknown");
			}
		} else {
			ui->last_reg_height->setText("not valid");
			ui->valid_thru_height->setText("not valid");
		}

		
			QString parentkey   = QString::fromStdString( reply["parentkey"].get<json::string_t>() );
			QString safekey     = QString::fromStdString( reply["safekey"].get<json::string_t>() );
			QString safeheight  = QString::fromStdString( reply["safeheight"].get<json::string_t>() );
			QString SAFE_address  = QString::fromStdString( reply["SAFE_address"].get<json::string_t>() );
			
			ui->parentkey->setText(parentkey);
			ui->safekey->setText(safekey);
			ui->safeheight->setText(safeheight);
			ui->safeaddress->setText(SAFE_address);
	} else {
			ui->balance->setText("not configured");
			ui->balance_usd->setText("not configured");
			ui->collateral->setText("not configured");
			ui->collateral_usd->setText("not configured");
			ui->tier->setText("not configured");
			ui->is_valid->setText("not configured");
			ui->errors->setText("not configured");
			ui->last_reg_height->setText("not configured");
			ui->valid_thru_height->setText("not configured");
			ui->parentkey->setText("not configured");
			ui->safekey->setText("not configured");
			ui->safeheight->setText("not configured");
			ui->safeaddress->setText("not configured");
	}
        });


        // Get network info
        payload = {
            {"jsonrpc", "1.0"},
            {"id", "someid"},
            {"method", "getnetworkinfo"}
        };

        conn->doRPCIgnoreError(payload, [=](const json& reply) {
            QString clientname    = QString::fromStdString( reply["subversion"].get<json::string_t>() );

            ui->clientname->setText(clientname);
        });


        // Call to see if the blockchain is syncing. 
        payload = {
            {"jsonrpc", "1.0"},
            {"id", "someid"},
            {"method", "getblockchaininfo"}
        };

        conn->doRPCIgnoreError(payload, [=](const json& reply) {
            auto progress    = reply["verificationprogress"].get<double>();
            bool isSyncing   = progress < 0.9999; // 99.99%
            int  blockNumber = reply["blocks"].get<json::number_unsigned_t>();

            int estimatedheight = 0;
            if (reply.find("estimatedheight") != reply.end()) {
                estimatedheight = reply["estimatedheight"].get<json::number_unsigned_t>();
            }

            Settings::getInstance()->setSyncing(isSyncing);
            Settings::getInstance()->setBlockNumber(blockNumber);

            // Update safecoind tab if it exists
                if (isSyncing) {
                    QString txt = QString::number(blockNumber);
                    if (estimatedheight > 0) {
                        txt = txt % " / ~" % QString::number(estimatedheight);
                        // If estimated height is available, then use the download blocks 
                        // as the progress instead of verification progress.
                        progress = (double)blockNumber / (double)estimatedheight;
                    }
                    txt = txt %  " ( " % QString::number(progress * 100, 'f', 2) % "% )";
                    ui->blockheight->setText(txt);
                    ui->heightLabel->setText(QObject::tr("Downloading blocks"));
                } else {
                    // If syncing is finished, we may have to remove the fastsync
                    // flag from safecoin.conf
                    if (getConnection() != nullptr && getConnection()->config->fastsync) {
                        getConnection()->config->fastsync = false;
                        Settings::removeFromZcashConf(Settings::getInstance()->getZcashdConfLocation(), 
                                                        "fastsync");
                    }

                    ui->blockheight->setText(QString::number(blockNumber));
                    ui->heightLabel->setText(QObject::tr("Block height"));
                }

            // Update the status bar
            QString statusText = QString() %
                (isSyncing ? QObject::tr("Syncing") : QObject::tr("Connected")) %
                " (" %
                (Settings::getInstance()->isTestnet() ? QObject::tr("testnet:") : "") %
                QString::number(blockNumber) %
                (isSyncing ? ("/" % QString::number(progress*100, 'f', 2) % "%") : QString()) %
                ") SAFE=$" % QString::number( (double) Settings::getInstance()->getZECPrice() );
            main->statusLabel->setText(statusText);   

            // Update the balances view to show a warning if the node is still syncing
            ui->lblSyncWarning->setVisible(isSyncing);
            ui->lblSyncWarningReceive->setVisible(isSyncing);

            auto zecPrice = Settings::getInstance()->getUSDFromZecAmount(1);
            QString tooltip;
            if (connections > 0) {
                tooltip = QObject::tr("Connected to safecoind");
            }
            else {
                tooltip = QObject::tr("safecoind has no peer connections");
            }
            tooltip = tooltip % "(v " % QString::number(Settings::getInstance()->getZcashdVersion()) % ")";

            if (!zecPrice.isEmpty()) {
                tooltip = "1 " % Settings::getTokenName() % " = " % zecPrice % "\n" % tooltip;
            }
            main->statusLabel->setToolTip(tooltip);
            main->statusIcon->setToolTip(tooltip);
        });

    }, [=](QNetworkReply* reply, const json&) {
        // safecoind has probably disappeared.
        this->noConnection();

        // Prevent multiple dialog boxes, because these are called async
        static bool shown = false;
        if (!shown && prevCallSucceeded) { // show error only first time
            shown = true;
            QMessageBox::critical(main, QObject::tr("Connection Error"), QObject::tr("There was an error connecting to safecoind. The error was") + ": \n\n"
                + reply->errorString(), QMessageBox::StandardButton::Ok);
            shown = false;
        }

        prevCallSucceeded = false;
    });
}

void RPC::refreshAddresses() {
    if  (conn == nullptr) 
        return noConnection();
    
    auto newzaddresses = new QList<QString>();

    getZAddresses([=] (json reply) {
        for (auto& it : reply.get<json::array_t>()) {   
            auto addr = QString::fromStdString(it.get<json::string_t>());
            newzaddresses->push_back(addr);
        }

        delete zaddresses;
        zaddresses = newzaddresses;

        // Refresh the sent and received txs from all these z-addresses
        refreshSentZTrans();
        refreshReceivedZTrans(*zaddresses);
    });

    
    auto newtaddresses = new QList<QString>();
    getTAddresses([=] (json reply) {
        for (auto& it : reply.get<json::array_t>()) {   
            auto addr = QString::fromStdString(it.get<json::string_t>());
            if (Settings::isTAddress(addr))
                newtaddresses->push_back(addr);
        }

        delete taddresses;
        taddresses = newtaddresses;

        // If there are no t Addresses, create one
        newTaddr([=] (json reply) {
            // What if taddress gets deleted before this executes?
            taddresses->append(QString::fromStdString(reply.get<json::string_t>()));
        });
    });
}

// Function to create the data model and update the views, used below.
void RPC::updateUI(bool anyUnconfirmed) {    
    ui->unconfirmedWarning->setVisible(anyUnconfirmed);

    // Update balances model data, which will update the table too
    balancesTableModel->setNewData(allBalances, utxos);

    // Update from address
    main->updateFromCombo();
};

// Function to process reply of the listunspent and z_listunspent API calls, used below.
bool RPC::processUnspent(const json& reply, QMap<QString, double>* balancesMap, QList<UnspentOutput>* newUtxos) {
    bool anyUnconfirmed = false;
    for (auto& it : reply.get<json::array_t>()) {
        QString qsAddr = QString::fromStdString(it["address"]);
        auto confirmations = it["confirmations"].get<json::number_unsigned_t>();
        if (confirmations == 0) {
            anyUnconfirmed = true;
        }

        newUtxos->push_back(
            UnspentOutput{ qsAddr, QString::fromStdString(it["txid"]),
                            Settings::getDecimalString(it["amount"].get<json::number_float_t>()),
                            (int)confirmations, it["spendable"].get<json::boolean_t>() });

        (*balancesMap)[qsAddr] = (*balancesMap)[qsAddr] + it["amount"].get<json::number_float_t>();
    }
    return anyUnconfirmed;
};

/**
 * Refresh the turnstile migration status
 */
void RPC::refreshMigration() {
    // Turnstile migration is only supported in safecoind v2.0.5 and above
    if (Settings::getInstance()->getZcashdVersion() < 2000552)
        return;

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_getmigrationstatus"},
    };
    
    conn->doRPCWithDefaultErrorHandling(payload, [=](json reply) {
        this->migrationStatus.available = true;
        this->migrationStatus.enabled   = reply["enabled"].get<json::boolean_t>();
        this->migrationStatus.saplingAddress = QString::fromStdString(reply["destination_address"]);
        this->migrationStatus.unmigrated = QString::fromStdString(reply["unmigrated_amount"]).toDouble();
        this->migrationStatus.migrated = QString::fromStdString(reply["finalized_migrated_amount"]).toDouble();

        QList<QString> ids;
        for (auto& it : reply["migration_txids"].get<json::array_t>()) {
            ids.push_back(QString::fromStdString(it.get<json::string_t>()));
        }
        this->migrationStatus.txids = ids;
    });
}

void RPC::setMigrationStatus(bool enabled) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_setmigration"},
        {"params", {enabled}}  
    };

    conn->doRPCWithDefaultErrorHandling(payload, [=](json) {
        // Ignore return value.
    });
}



void RPC::refreshBalances() {    
    if  (conn == nullptr) 
        return noConnection();

    // 1. Get the Balances
    getBalance([=] (json reply) {    
        auto balT      = QString::fromStdString(reply["transparent"]).toDouble();
        auto balZ      = QString::fromStdString(reply["private"]).toDouble();
        auto balTotal  = QString::fromStdString(reply["total"]).toDouble();


        AppDataModel::getInstance()->setBalances(balT, balZ);

        ui->balSheilded   ->setText(Settings::getZECDisplayFormat(balZ));
        ui->balTransparent->setText(Settings::getZECDisplayFormat(balT));
        ui->balTotal      ->setText(Settings::getZECDisplayFormat(balTotal));


        ui->balSheilded   ->setToolTip(Settings::getZECDisplayFormat(balZ));
        ui->balTransparent->setToolTip(Settings::getZECDisplayFormat(balT));
        ui->balTotal      ->setToolTip(Settings::getZECDisplayFormat(balTotal));

        ui->balUSDTotal      ->setText(Settings::getUSDFromZecAmount(balTotal));
        ui->balUSDTotal      ->setToolTip(Settings::getUSDFromZecAmount(balTotal));
    });

    // 2. Get the UTXOs
    // First, create a new UTXO list. It will be replacing the existing list when everything is processed.
    auto newUtxos = new QList<UnspentOutput>();
    auto newBalances = new QMap<QString, double>();

    // Call the Transparent and Z unspent APIs serially and then, once they're done, update the UI
    getTransparentUnspent([=] (json reply) {
        auto anyTUnconfirmed = processUnspent(reply, newBalances, newUtxos);

        getZUnspent([=] (json reply) {
            auto anyZUnconfirmed = processUnspent(reply, newBalances, newUtxos);

            // Swap out the balances and UTXOs
            delete allBalances;
            delete utxos;

            allBalances = newBalances;
            utxos       = newUtxos;

            updateUI(anyTUnconfirmed || anyZUnconfirmed);

            main->balancesReady();
        });        
    });
}

void RPC::refreshTransactions() {    
    if  (conn == nullptr) 
        return noConnection();

    getTransactions([=] (json reply) {
        QList<TransactionItem> txdata;

        for (auto& it : reply.get<json::array_t>()) {  
            double fee = 0;
            if (!it["fee"].is_null()) {
                fee = it["fee"].get<json::number_float_t>();
            }

            QString address = (it["address"].is_null() ? "" : QString::fromStdString(it["address"]));

            TransactionItem tx{
                QString::fromStdString(it["category"]),
                (qint64)it["time"].get<json::number_unsigned_t>(),
                address,
                QString::fromStdString(it["txid"]),
                it["amount"].get<json::number_float_t>() + fee,
                static_cast<long>(it["confirmations"].get<json::number_unsigned_t>()),
                "", "" };

            txdata.push_back(tx);
            if (!address.isEmpty())
                usedAddresses->insert(address, true);
        }

        // Update model data, which updates the table view
        transactionsTableModel->addTData(txdata);        
    });
}

// Read sent Z transactions from the file.
void RPC::refreshSentZTrans() {
    if  (conn == nullptr) 
        return noConnection();

    auto sentZTxs = SentTxStore::readSentTxFile();

    // If there are no sent z txs, then empty the table. 
    // This happens when you clear history.
    if (sentZTxs.isEmpty()) {
        transactionsTableModel->addZSentData(sentZTxs);
        return;
    }

    QList<QString> txids;

    for (auto sentTx: sentZTxs) {
        txids.push_back(sentTx.txid);
    }

    // Look up all the txids to get the confirmation count for them. 
    conn->doBatchRPC<QString>(txids,
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
                    sentTx.confirmations = j["confirmations"].get<json::number_integer_t>();
            }
            
            transactionsTableModel->addZSentData(newSentZTxs);
            delete txidList;
        }
     );
}

void RPC::addNewTxToWatch(const QString& newOpid, WatchedTx wtx) {    
    watchingOps.insert(newOpid, wtx);

    watchTxStatus();
}

/**
 * Execute a transaction with the standard UI. i.e., standard status bar message and standard error
 * handling
 */
void RPC::executeStandardUITransaction(Tx tx) {
    executeTransaction(tx, 
        [=] (QString opid) {
            ui->statusBar->showMessage(QObject::tr("Computing Tx: ") % opid);
        },
        [=] (QString, QString txid) { 
            ui->statusBar->showMessage(Settings::txidStatusMessage + " " + txid);
        },
        [=] (QString opid, QString errStr) {
            ui->statusBar->showMessage(QObject::tr(" Tx ") % opid % QObject::tr(" failed"), 15 * 1000);

            if (!opid.isEmpty())
                errStr = QObject::tr("The transaction with id ") % opid % QObject::tr(" failed. The error was") + ":\n\n" + errStr; 

            QMessageBox::critical(main, QObject::tr("Transaction Error"), errStr, QMessageBox::Ok);            
        }
    );
}

// Execute a transaction!
void RPC::executeTransaction(Tx tx, 
        const std::function<void(QString opid)> submitted,
        const std::function<void(QString opid, QString txid)> computed,
        const std::function<void(QString opid, QString errStr)> error) {
    // First, create the json params
    json params = json::array();
    fillTxJsonParams(params, tx);
    std::cout << std::setw(2) << params << std::endl;

    sendZTransaction(params, [=](const json& reply) {
        QString opid = QString::fromStdString(reply.get<json::string_t>());

        // And then start monitoring the transaction
        addNewTxToWatch( opid, WatchedTx { opid, tx, computed, error} );
        submitted(opid);
    },
    [=](QString errStr) {
        error("", errStr);
    });
}


void RPC::watchTxStatus() {
    if  (conn == nullptr) 
        return noConnection();

    // Make an RPC to load pending operation statues
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_getoperationstatus"},
    };

    conn->doRPCIgnoreError(payload, [=] (const json& reply) {
        // There's an array for each item in the status
        for (auto& it : reply.get<json::array_t>()) {  
            // If we were watching this Tx and its status became "success", then we'll show a status bar alert
            QString id = QString::fromStdString(it["id"]);
            if (watchingOps.contains(id)) {
                // And if it ended up successful
                QString status = QString::fromStdString(it["status"]);
                main->loadingLabel->setVisible(false);

                if (status == "success") {
                    auto txid = QString::fromStdString(it["result"]["txid"]);
                    
                    SentTxStore::addToSentTx(watchingOps[id].tx, txid);

                    auto wtx = watchingOps[id];
                    watchingOps.remove(id);
                    wtx.completed(id, txid);

                    // Refresh balances to show unconfirmed balances                    
                    refresh(true);
                } else if (status == "failed") {
                    // If it failed, then we'll actually show a warning. 
                    auto errorMsg = QString::fromStdString(it["error"]["message"]);

                    auto wtx = watchingOps[id];
                    watchingOps.remove(id);
                    wtx.error(id, errorMsg);
                } 
            }

            if (watchingOps.isEmpty()) {
                txTimer->start(Settings::updateSpeed);
            } else {
                txTimer->start(Settings::quickUpdateSpeed);
            }
        }

        // If there is some op that we are watching, then show the loading bar, otherwise hide it
        if (watchingOps.empty()) {
            main->loadingLabel->setVisible(false);
        } else {
            main->loadingLabel->setVisible(true);
            main->loadingLabel->setToolTip(QString::number(watchingOps.size()) + QObject::tr(" tx computing. This can take several minutes."));
        }
    });
}

void RPC::checkForUpdate(bool silent) {
    if  (conn == nullptr) 
        return noConnection();

    QUrl cmcURL("https://api.github.com/repos/Fair-Exchange/safecoinwallet/releases");

    QNetworkRequest req;
    req.setUrl(cmcURL);
    
    QNetworkReply *reply = conn->restclient->get(req);

    QObject::connect(reply, &QNetworkReply::finished, [=] {
        reply->deleteLater();

        try {
            if (reply->error() == QNetworkReply::NoError) {

                auto releases = QJsonDocument::fromJson(reply->readAll()).array();
                QVersionNumber maxVersion(0, 0, 0);

                for (QJsonValue rel : releases) {
                    if (!rel.toObject().contains("tag_name"))
                        continue;

                    QString tag = rel.toObject()["tag_name"].toString();
                    if (tag.startsWith("v"))
                        tag = tag.right(tag.length() - 1);

                    if (!tag.isEmpty()) {
                        auto v = QVersionNumber::fromString(tag);
                        if (v > maxVersion)
                            maxVersion = v;
                    }
                }

                auto currentVersion = QVersionNumber::fromString(APP_VERSION);
                
                // Get the max version that the user has hidden updates for
                QSettings s;
                auto maxHiddenVersion = QVersionNumber::fromString(s.value("update/lastversion", "0.0.0").toString());

                qDebug() << "Version check: Current " << currentVersion << ", Available " << maxVersion;

                if (maxVersion > currentVersion && (!silent || maxVersion > maxHiddenVersion)) {
                    auto ans = QMessageBox::information(main, QObject::tr("Update Available"), 
                        QObject::tr("A new release v%1 is available! You have v%2.\n\nWould you like to visit the releases page?")
                            .arg(maxVersion.toString())
                            .arg(currentVersion.toString()),
                        QMessageBox::Yes, QMessageBox::Cancel);
                    if (ans == QMessageBox::Yes) {
                        QDesktopServices::openUrl(QUrl("https://github.com/Fair-Exchange/safecoinwallet/releases"));
                    } else {
                        // If the user selects cancel, don't bother them again for this version
                        s.setValue("update/lastversion", maxVersion.toString());
                    }
                } else {
                    if (!silent) {
                        QMessageBox::information(main, QObject::tr("No updates available"), 
                            QObject::tr("You already have the latest release v%1")
                                .arg(currentVersion.toString()));
                    }
                } 
            }
        }
        catch (...) {
            // If anything at all goes wrong, just set the price to 0 and move on.
            qDebug() << QString("Caught something nasty");
        }       
    });
}

// Get the ZEC->USD price from coinmarketcap using their API
void RPC::refreshZECPrice() {
    if  (conn == nullptr)
        return noConnection();

    QUrl cmcURL("https://api.coinmarketcap.com/v1/ticker/safecoin/");

    QNetworkRequest req;
    req.setUrl(cmcURL);

    QNetworkReply *reply = conn->restclient->get(req);

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
                if (item["symbol"].get<json::string_t>() == Settings::getTokenName().toStdString()) {
                    QString price = QString::fromStdString(item["price_usd"].get<json::string_t>());
                    qDebug() << Settings::getTokenName() << " Price=" << price;
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

void RPC::shutdownZcashd() {
    // Shutdown embedded safecoind if it was started
    if (ezcashd == nullptr || ezcashd->processId() == 0 || conn == nullptr) {
        // No safecoind running internally, just return
        return;
    }

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "stop"}
    };
    
    conn->doRPCWithDefaultErrorHandling(payload, [=](auto) {});
    conn->shutdown();

    QDialog d(main);
    Ui_ConnectionDialog connD;
    connD.setupUi(&d);
    connD.topIcon->setBasePixmap(QIcon(":/icons/res/icon.ico").pixmap(256, 256));
    connD.status->setText(QObject::tr("Please wait for SafecoinWallet to exit"));
    connD.statusDetail->setText(QObject::tr("Waiting for safecoind to exit"));

    QTimer waiter(main);

    // We capture by reference all the local variables because of the d.exec() 
    // below, which blocks this function until we exit. 
    int waitCount = 0;
    QObject::connect(&waiter, &QTimer::timeout, [&] () {
        waitCount++;

        if ((ezcashd->atEnd() && ezcashd->processId() == 0) ||
            waitCount > 30 || 
            conn->config->zcashDaemon)  {   // If safecoind is daemon, then we don't have to do anything else
            qDebug() << "Ended";
            waiter.stop();
            QTimer::singleShot(1000, [&]() { d.accept(); });
        } else {
            qDebug() << "Not ended, continuing to wait...";
        }
    });
    waiter.start(1000);

    // Wait for the safecoin process to exit.
    if (!Settings::getInstance()->isHeadless()) {
        d.exec(); 
    } else {
        while (waiter.isActive()) {
            QCoreApplication::processEvents();

            QThread::sleep(1);
        }
    }
}


// Fetch the Z-board topics list
void RPC::getZboardTopics(std::function<void(QMap<QString, QString>)> cb) {
    if (conn == nullptr)
        return noConnection();

    QUrl cmcURL("http://z-board.net/listTopics");

    QNetworkRequest req;
    req.setUrl(cmcURL);

    QNetworkReply *reply = conn->restclient->get(req);

    QObject::connect(reply, &QNetworkReply::finished, [=] {
        reply->deleteLater();

        try {
            if (reply->error() != QNetworkReply::NoError) {
                auto parsed = json::parse(reply->readAll(), nullptr, false);
                if (!parsed.is_discarded() && !parsed["error"]["message"].is_null()) {
                    qDebug() << QString::fromStdString(parsed["error"]["message"]);
                }
                else {
                    qDebug() << reply->errorString();
                }
                return;
            }

            auto all = reply->readAll();

            auto parsed = json::parse(all, nullptr, false);
            if (parsed.is_discarded()) {
                return;
            }

            QMap<QString, QString> topics;
            for (const json& item : parsed["topics"].get<json::array_t>()) {
                if (item.find("addr") == item.end() || item.find("topicName") == item.end())
                    return;

                QString addr  = QString::fromStdString(item["addr"].get<json::string_t>());
                QString topic = QString::fromStdString(item["topicName"].get<json::string_t>());
                
                topics.insert(topic, addr);
            }

            cb(topics);
        }
        catch (...) {
            // If anything at all goes wrong, just set the price to 0 and move on.
            qDebug() << QString("Caught something nasty");
        }
    });
}

/** 
 * Get a Sapling address from the user's wallet
 */ 
QString RPC::getDefaultSaplingAddress() {
    for (QString addr: *zaddresses) {
        if (Settings::getInstance()->isSaplingAddress(addr))
            return addr;
    }

    return QString();
}

QString RPC::getDefaultTAddress() {
    if (getAllTAddresses()->length() > 0)
        return getAllTAddresses()->at(0);
    else 
        return QString();
}
