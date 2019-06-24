#include "senttxstore.h"
#include "settings.h"

/// Get the location of the app data file to be written. 
QString SentTxStore::writeableFile() {
    auto filename = QStringLiteral("senttxstore.dat");

    auto dir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!dir.exists())
        QDir().mkpath(dir.absolutePath());

    if (Settings::getInstance()->isTestnet()) {
        return dir.filePath("testnet-" % filename);
    } else {
        return dir.filePath(filename);
    }
}

// delete the sent history. 
void SentTxStore::deleteHistory() {
    QFile data(writeableFile());
    data.remove();
    data.close();
}

QList<TransactionItem> SentTxStore::readSentTxFile() {
    QFile data(writeableFile());
    if (!data.exists()) {
        return QList<TransactionItem>();
    }
    
    QJsonDocument jsonDoc;
    
    data.open(QFile::ReadOnly);    
    jsonDoc = QJsonDocument::fromJson(data.readAll());
    data.close();

    QList<TransactionItem> items;

    for (auto i : jsonDoc.array()) {
        auto sentTx = i.toObject();
        TransactionItem t{"send", (qint64)sentTx["datetime"].toVariant().toLongLong(), 
                          sentTx["address"].toString(), 
                          sentTx["txid"].toString(), 
                          sentTx["amount"].toDouble() + sentTx["fee"].toDouble(), 
                          0, sentTx["from"].toString(), ""};
        items.push_back(t);
    }

    return items;
}

void SentTxStore::addToSentTx(Tx tx, QString txid) {
    // Save transactions only if the settings are allowed
    if (!Settings::getInstance()->getSaveZtxs())
        return;

    // Also, only store outgoing txs where the from address is a z-Addr. Else, regular zcashd 
    // stores it just fine
    if (!tx.fromAddr.startsWith("z")) 
        return;

    QFile data(writeableFile());
    QJsonDocument jsonDoc;

    // If data doesn't exist, then create a blank one
    if (!data.exists()) {
        QJsonArray a;
        jsonDoc.setArray(a);

        QFile newFile(writeableFile());
        newFile.open(QFile::WriteOnly);
        newFile.write(jsonDoc.toJson());
        newFile.close();
    } else {
        data.open(QFile::ReadOnly);    
        jsonDoc = QJsonDocument().fromJson(data.readAll());
        data.close();
    }

    // Calculate total amount in this tx
    double totalAmount = 0;
    for (auto i : tx.toAddrs) {
        totalAmount += i.amount;
    }

    QString toAddresses;
    if (tx.toAddrs.length() == 1) {
        toAddresses = tx.toAddrs[0].addr;
    } else {
        // Concatenate all the toAddresses
        for (auto a : tx.toAddrs) {
            toAddresses += a.addr % "(" % Settings::getZECDisplayFormat(a.amount) % ")  ";
        }
    }

    auto list = jsonDoc.array();
    QJsonObject txItem;
    txItem["type"]      = "sent";
    txItem["from"]      = tx.fromAddr;
    txItem["datetime"]  = QDateTime::currentMSecsSinceEpoch() / (qint64)1000;
    txItem["address"]   = toAddresses;
    txItem["txid"]      = txid;
    txItem["amount"]    = -totalAmount;
    txItem["fee"]       = -tx.fee;
    // TODO: store all outgoing memos
    txItem["memo"]      = tx.toAddrs[0].txtMemo;
    list.append(txItem);

    jsonDoc.setArray(list);

    QFile writer(writeableFile());
    if (writer.open(QFile::WriteOnly | QFile::Truncate)) {
        writer.write(jsonDoc.toJson());
    } 
    writer.close();
}
