#include "zcashdrpc.h"

ZcashdRPC::ZcashdRPC() {

}

ZcashdRPC::~ZcashdRPC() {
    delete conn;
}

void ZcashdRPC::setConnection(Connection* c) {
    if (conn) {
        delete conn;
    }
    
    conn = c;
}

bool ZcashdRPC::haveConnection() {
    return conn != nullptr;
}

void ZcashdRPC::fetchTAddresses(const std::function<void(json)>& cb) {
    if (conn == nullptr)
        return;

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "getaddressesbyaccount"},
        {"params", {""}}
    };

    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void ZcashdRPC::fetchZAddresses(const std::function<void(json)>& cb) {
    if (conn == nullptr)
        return;

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_listaddresses"},
    };

    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void ZcashdRPC::fetchTransparentUnspent(const std::function<void(json)>& cb) {
    if (conn == nullptr)
        return;

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "listunspent"},
        {"params", {0}}             // Get UTXOs with 0 confirmations as well.
    };

    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void ZcashdRPC::fetchZUnspent(const std::function<void(json)>& cb) {
    if (conn == nullptr)
        return;

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_listunspent"},
        {"params", {0}}             // Get UTXOs with 0 confirmations as well.
    };

    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void ZcashdRPC::createNewZaddr(bool sapling, const std::function<void(json)>& cb) {
    if (conn == nullptr)
        return;

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_getnewaddress"},
        {"params", { sapling ? "sapling" : "sprout" }},
    };
    
    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void ZcashdRPC::createNewTaddr(const std::function<void(json)>& cb) {
    if (conn == nullptr)
        return;

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "getnewaddress"},
    };

    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void ZcashdRPC::fetchZPrivKey(QString addr, const std::function<void(json)>& cb) {
    if (conn == nullptr)
        return;

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_exportkey"},
        {"params", { addr.toStdString() }},
    };
    
    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void ZcashdRPC::fetchTPrivKey(QString addr, const std::function<void(json)>& cb) {
    if (conn == nullptr)
        return;

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "dumpprivkey"},
        {"params", { addr.toStdString() }},
    };
    
    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void ZcashdRPC::importZPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb) {
    if (conn == nullptr)
        return;

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_importkey"},
        {"params", { addr.toStdString(), (rescan? "yes" : "no") }},
    };
    
    conn->doRPCWithDefaultErrorHandling(payload, cb);
}


void ZcashdRPC::importTPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb) {
    if (conn == nullptr)
        return;

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "importprivkey"},
        {"params", { addr.toStdString(), (rescan? "yes" : "no") }},
    };
    
    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void ZcashdRPC::validateAddress(QString address, const std::function<void(json)>& cb) {
    if (conn == nullptr)
        return;

    QString method = Settings::isZAddress(address) ? "z_validateaddress" : "validateaddress";

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", method.toStdString() },
        {"params", { address.toStdString() } },
    };
    
    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void ZcashdRPC::fetchBalance(const std::function<void(json)>& cb) {
    if (conn == nullptr)
        return;

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_gettotalbalance"},
        {"params", {0}}             // Get Unconfirmed balance as well.
    };

    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void ZcashdRPC::fetchTransactions(const std::function<void(json)>& cb) {
    if (conn == nullptr)
        return;

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "listtransactions"}
    };

    conn->doRPCWithDefaultErrorHandling(payload, cb);
}

void ZcashdRPC::sendZTransaction(json params, const std::function<void(json)>& cb, 
    const std::function<void(QString)>& err) {
    if (conn == nullptr)
        return;

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

void ZcashdRPC::fetchInfo(const std::function<void(json)>& cb, 
    const std::function<void(QNetworkReply*, const json&)>&  err) {
    if (conn == nullptr)
        return;

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "getinfo"}
    };

    conn->doRPC(payload, cb, err); 
}

void ZcashdRPC::fetchBlockchainInfo(const std::function<void(json)>& cb) {
    if (conn == nullptr)
        return;

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "getblockchaininfo"}
    };

    conn->doRPCIgnoreError(payload, cb);
}

void ZcashdRPC::fetchNetSolOps(const std::function<void(qint64)> cb) {
    if (conn == nullptr)
        return;

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "getnetworksolps"}
    };

    conn->doRPCIgnoreError(payload, [=](const json& reply) {
        qint64 solrate = reply.get<json::number_unsigned_t>();
        cb(solrate);
    });
}

void ZcashdRPC::fetchMigrationStatus(const std::function<void(json)>& cb) {
    if (conn == nullptr)
        return;

    if (!Settings::getInstance()->isSaplingActive()) {
        return;
    }

    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_getmigrationstatus"},
    };
    
    conn->doRPCWithDefaultErrorHandling(payload, cb);
}


void ZcashdRPC::setMigrationStatus(bool enabled) {
    if (conn == nullptr)
        return;

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


/**
 * Method to get all the private keys for both z and t addresses. It will make 2 batch calls,
 * combine the result, and call the callback with a single list containing both the t-addr and z-addr
 * private keys
 */ 
void ZcashdRPC::fetchAllPrivKeys(const std::function<void(QList<QPair<QString, QString>>)> cb) {
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

void ZcashdRPC::fetchOpStatus(const std::function<void(json)>& cb) {
    if (conn == nullptr)
        return;

    // Make an RPC to load pending operation statues
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "z_getoperationstatus"},
    };

    conn->doRPCIgnoreError(payload, cb);
}

void ZcashdRPC::fetchReceivedTTrans(QList<QString> txids, QList<TransactionItem> sentZTxs,
                const std::function<void(QList<TransactionItem>)> txdataFn) {
    if (conn == nullptr)
        return;

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
            
            txdataFn(newSentZTxs);
            delete txidList;
        }
     );
}


// Refresh received z txs by calling z_listreceivedbyaddress/gettransaction
void ZcashdRPC::fetchReceivedZTrans(QList<QString> zaddrs, const std::function<void(QString)> usedAddrFn,
        const std::function<void(QList<TransactionItem>)> txdataFn) {
    if (conn == nullptr)
        return;


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
                    usedAddrFn(zaddr);

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

                    txdataFn(txdata);

                    // Cleanup both responses;
                    delete zaddrTxids;
                    delete txidDetails;
                }
            );
        }
    );
} 
