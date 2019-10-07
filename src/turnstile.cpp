#include "turnstile.h"
#include "mainwindow.h"
#include "balancestablemodel.h"
#include "rpc.h"
#include "settings.h"
#include "ui_migration.h"


using json = nlohmann::json;

Turnstile::Turnstile(RPC* _rpc, MainWindow* mainwindow) {
    this->rpc = _rpc;
    this->mainwindow = mainwindow;
}


void printPlan(QList<TurnstileMigrationItem> plan) {
    for (auto item : plan) {
        //qDebug() << item.fromAddr << item.intTAddr 
        //            << item.destAddr << item.amount << item.blockNumber << item.status;
    }
}

QString Turnstile::writeableFile() {
    auto filename = QStringLiteral("turnstilemigrationplan.dat");

    auto dir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!dir.exists())
        QDir().mkpath(dir.absolutePath());

    if (Settings::getInstance()->isTestnet()) {
        return dir.filePath("testnet-" % filename);
    } else {
        return dir.filePath(filename);
    }
}

void Turnstile::removeFile() {
    QFile(writeableFile()).remove();
}

// Data stream write/read methods for migration items
QDataStream &operator<<(QDataStream& ds, const TurnstileMigrationItem& item) {
    return ds << QString("v1") << item.fromAddr << item.intTAddr 
                 << item.destAddr << item.amount << item.blockNumber << item.status;
}

QDataStream &operator>>(QDataStream& ds, TurnstileMigrationItem& item) {
    QString version;
    return ds >> version >> item.fromAddr >> item.intTAddr 
                 >> item.destAddr >> item.amount >> item.blockNumber >> item.status;
}

void Turnstile::writeMigrationPlan(QList<TurnstileMigrationItem> plan) {
    //qDebug() << QString("Writing plan");
    printPlan(plan);

    QFile file(writeableFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate);
    QDataStream out(&file);   // we will serialize the data into the file
    out << plan;
    file.close();
}

QList<TurnstileMigrationItem> Turnstile::readMigrationPlan() {
    QFile file(writeableFile());
    
    QList<TurnstileMigrationItem> plan;
    if (!file.exists()) return plan;

    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);    // read the data serialized from the file
    in >> plan; 

    file.close();

    // Sort to see when the next step is.
    std::sort(plan.begin(), plan.end(), [&] (auto a, auto b) {
        return a.blockNumber < b.blockNumber;
    });        

    return plan;
}

void Turnstile::planMigration(QString zaddr, QString destAddr, int numsplits, int numBlocks) {
    // First, get the balance and split up the amounts
    auto bal = rpc->getModel()->getAllBalances().value(zaddr);
    auto splits = splitAmount(bal, numsplits);

    // Then, generate an intermediate t-address for each part using getBatchRPC
    rpc->getConnection()->doBatchRPC<double>(splits,
        [=] (double /*unused*/) {
            json payload = {
                {"jsonrpc", "1.0"},
                {"id", "someid"},
                {"method", "getnewaddress"},
            };
            return payload;
        },
        [=] (QMap<double, json>* newAddrs) {
            // Get block numbers
            auto curBlock = Settings::getInstance()->getBlockNumber();
            auto blockNumbers = getBlockNumbers(curBlock, curBlock + numBlocks, splits.size());

            // Assign the amounts to the addresses.
            QList<TurnstileMigrationItem> migItems;
            
            for (int i=0; i < splits.size(); i++) {
                auto tAddr = newAddrs->values()[i].get<json::string_t>();
                auto item = TurnstileMigrationItem { zaddr, QString::fromStdString(tAddr), destAddr,
                                                     blockNumbers[i], splits[i], 
                                                     TurnstileMigrationItemStatus::NotStarted };
                migItems.push_back(item);
            }

            // The first migration is shifted to the current block, so the user sees something 
            // happening immediately
            if (migItems.empty()) {
                // Show error and abort
                QMessageBox::warning(mainwindow, 
                    QObject::tr("Locked funds"), 
                    QObject::tr("Could not initiate migration.\nYou either have unconfirmed funds or the balance is too low for an automatic migration."));
                return;
            }

            migItems[0].blockNumber = curBlock;

            std::sort(migItems.begin(), migItems.end(), [&] (auto a, auto b) {
                return a.blockNumber < b.blockNumber;
            });        

            writeMigrationPlan(migItems);
            rpc->refresh(true);    // Force refresh, to start the migration immediately
        }
    );
}


QList<int> Turnstile::getBlockNumbers(int start, int end, int count) {
    QList<int> blocks;
    // Generate 'count' numbers between [start, end]
    for (int i=0; i < count; i++) {
        auto blk = (std::rand() % (end - start)) + start;
        blocks.push_back(blk);
    }

    return blocks;
}

    // Need at least 0.0005 ZEC for this
double Turnstile::minMigrationAmount = 0.0005;

QList<double> Turnstile::splitAmount(double amount, int parts) {
    QList<double> amounts;

    if (amount < minMigrationAmount)
        return amounts;
    
    fillAmounts(amounts, amount, parts);
    //qDebug() << amounts;

    // Ensure they all add up!
    double sumofparts = 0;
    for (auto a : amounts) {
        sumofparts += a;
    }
    
    // Add the Tx fees
    sumofparts += amounts.size() * Settings::getMinerFee();

    return amounts;
}

void Turnstile::fillAmounts(QList<double>& amounts, double amount, int count) {
    if (count == 1 || amount < 0.01) {
        // Also account for the fees needed to send all these transactions
        auto actual = amount - (Settings::getMinerFee() * (amounts.size() + 1));

        amounts.push_back(actual);
        return;
    }

    // Get a random amount off the total amount and call recursively.
    // Multiply by hundred, because we'll operate on 0.01 ZEC minimum. We'll divide by 100 later on 
    // in this function.
    double curAmount = std::rand() % (int)std::floor(amount * 100);

    // Try to round it off
    auto places = (int)std::floor(std::log10(curAmount));
    if (places > 0) {
        auto a = std::pow(10, places);
        curAmount = std::floor(curAmount / a) * a;
    }

    // And divide by 100
    curAmount = curAmount / 100;

    if (curAmount > 0)
        amounts.push_back(curAmount);

    fillAmounts(amounts, amount - curAmount, count - 1);
}

QList<TurnstileMigrationItem>::Iterator
Turnstile::getNextStep(QList<TurnstileMigrationItem>& plan) {
    // Get to the next unexecuted step
    auto fnIsEligibleItem = [&] (auto item) {
        return     item.status == TurnstileMigrationItemStatus::NotStarted || 
                item.status == TurnstileMigrationItemStatus::SentToT;
    };

    // Find the next step
    auto nextStep = std::find_if(plan.begin(), plan.end(), fnIsEligibleItem);
    return nextStep;
}

bool Turnstile::isMigrationPresent() {
    auto plan = readMigrationPlan();
    return !plan.isEmpty();
}

ProgressReport Turnstile::getPlanProgress() {
    auto plan = readMigrationPlan();

    auto nextStep = getNextStep(plan);

    auto step = std::distance(plan.begin(), nextStep) * 2;    // 2 steps per item
    if (nextStep != plan.end() && 
        nextStep->status == TurnstileMigrationItemStatus::SentToT)
        step++;
    
    auto total = plan.size();

    auto nextBlock = nextStep == plan.end() ? 0 : nextStep->blockNumber;

    bool hasErrors = std::find_if(plan.begin(), plan.end(), [=] (auto i) {
        return  i.status == TurnstileMigrationItemStatus::NotEnoughBalance || 
                 i.status == TurnstileMigrationItemStatus::UnknownError;
    }) != plan.end();

    auto stepData = (nextStep == plan.end() ? std::prev(nextStep) : nextStep);

    return ProgressReport{(int)step, total*2, nextBlock, hasErrors, stepData->fromAddr, stepData->destAddr, stepData->intTAddr};
}

void Turnstile::executeMigrationStep() {
    // Do a step only if not syncing, else wait for the blockchain to catch up
    if (Settings::getInstance()->isSyncing())
        return;

    // Also, process payments only when the Payments UI is ready, otherwise
    // we might mess things up
    if (!mainwindow->isPaymentsReady())
        return;

    auto plan = readMigrationPlan();

    //qDebug() << QString("Executing step");
    printPlan(plan);

    // Get to the next unexecuted step
    auto fnIsEligibleItem = [&] (auto item) {
        return  item.status == TurnstileMigrationItemStatus::NotStarted || 
                item.status == TurnstileMigrationItemStatus::SentToT;
    };

    // Fn to find if there are any unconfirmed funds for this address.
    auto fnHasUnconfirmed = [=] (QString addr) {
        auto utxoset = rpc->getModel()->getUTXOs();
        return std::find_if(utxoset.begin(), utxoset.end(), [=] (auto utxo) {
                    return utxo.address == addr && utxo.confirmations == 0 && utxo.spendable;
                }) != utxoset.end();
    };

    // Find the next step
    auto nextStep = std::find_if(plan.begin(), plan.end(), fnIsEligibleItem);

    if (nextStep == plan.end()) 
        return; // Nothing to do    
    
    if (nextStep->blockNumber > Settings::getInstance()->getBlockNumber()) 
        return;

    // Is this the last step for this address?
    auto lastStep = std::find_if(std::next(nextStep), plan.end(), fnIsEligibleItem) == plan.end();

    // Execute this step
    if (nextStep->status == TurnstileMigrationItemStatus::NotStarted) {
        // Does this z addr have enough balance?
        if (fnHasUnconfirmed(nextStep->fromAddr)) {
            //qDebug() << QString("unconfirmed, waiting");
            return;
        }

        auto balance = rpc->getModel()->getAllBalances().value(nextStep->fromAddr);
        if (nextStep->amount > balance) {
            qDebug() << "Not enough balance!";
            nextStep->status = TurnstileMigrationItemStatus::NotEnoughBalance;
            writeMigrationPlan(plan);
            return;
        }

        auto to = ToFields{ nextStep->intTAddr, nextStep->amount, "", "" };

        // If this is the last step, then send the remaining amount instead of the actual amount.
        if (lastStep) {
            auto remainingAmount = balance - Settings::getMinerFee();
            if (remainingAmount > 0) {
                to.amount = remainingAmount;
            }
        }

        // Create the Tx
        auto tx = Tx{ nextStep->fromAddr, { to }, Settings::getMinerFee() };

        // And send it
        doSendTx(tx, [=] () {
            // Update status and write plan to disk
            nextStep->status = TurnstileMigrationItemStatus::SentToT;
            writeMigrationPlan(plan);
        });
    } else if (nextStep->status == TurnstileMigrationItemStatus::SentToT) {
        // First thing to do is check to see if the funds are confirmed. 
        // We'll check both the original sprout address and the intermediate t-addr for safety.
        if (fnHasUnconfirmed(nextStep->intTAddr) || fnHasUnconfirmed(nextStep->fromAddr)) {
            //qDebug() << QString("unconfirmed, waiting");
            return;
        }

        // Sometimes, we check too quickly, and the unspent UTXO is not updated yet, so we'll
        // double check to see if there is enough balance. 
        if (!rpc->getModel()->getAllBalances().keys().contains(nextStep->intTAddr)) {
            //qDebug() << QString("The intermediate t-address doesn't have balance, even though it seems to be confirmed");
            return;
        }

        // Send it to the final destination address.
        auto bal = rpc->getModel()->getAllBalances().value(nextStep->intTAddr);
        auto sendAmt = bal - Settings::getMinerFee();

        if (sendAmt < 0) {
            qDebug() << "Not enough balance!." << bal << ":" << sendAmt;
            nextStep->status = TurnstileMigrationItemStatus::NotEnoughBalance;
            writeMigrationPlan(plan);
            return;
        }
        
        QList<ToFields> to = { ToFields{ nextStep->destAddr, sendAmt, "", "" } };

        // Create the Tx
        auto tx = Tx{ nextStep->intTAddr, to, Settings::getMinerFee()};

        // And send it
        doSendTx(tx, [=] () {
            // Update status and write plan to disk
            nextStep->status = TurnstileMigrationItemStatus::SentToZS;
            writeMigrationPlan(plan);
        });
    }
}

void Turnstile::doSendTx(Tx tx, std::function<void(void)> cb) {
    rpc->executeTransaction(tx, [=] (QString opid) {
            mainwindow->ui->statusBar->showMessage(QObject::tr("Computing Tx: ") % opid);
        },
        [=] (QString /*opid*/, QString txid) { 
            mainwindow->ui->statusBar->showMessage(Settings::txidStatusMessage + " " + txid);
            cb();
        },
        [=] (QString opid, QString errStr) {
            mainwindow->ui->statusBar->showMessage(QObject::tr(" Tx ") % opid % QObject::tr(" failed"), 15 * 1000);

            if (!opid.isEmpty())
                errStr = QObject::tr("The transaction with id ") % opid % QObject::tr(" failed. The error was") + ":\n\n" + errStr; 

            QMessageBox::critical(mainwindow, QObject::tr("Transaction Error"), errStr, QMessageBox::Ok);            
        });
    
}


// Methods for zcashd native Migration
void Turnstile::showZcashdMigration(MainWindow* parent) {
    // If it is not enabled, don't show the dialog
    if (! parent->getRPC()->getMigrationStatus()->available)
        return;

    Ui_MigrationDialog md;
    QDialog d(parent);
    md.setupUi(&d);
    Settings::saveRestore(&d);

    MigrationTxns model(md.tblTxids, parent->getRPC()->getMigrationStatus()->txids);
    md.tblTxids->setModel(&model);

    // Table right click
    md.tblTxids->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(md.tblTxids, &QTableView::customContextMenuRequested, [=, &model] (QPoint pos) {
        QModelIndex index = md.tblTxids->indexAt(pos);
        if (index.row() < 0) return;

        QMenu menu(parent);
        QString txid = model.getTxid(index.row());

        menu.addAction("Copy txid", [=]() {
            QGuiApplication::clipboard()->setText(txid);
        });

        menu.addAction(QObject::tr("View on block explorer"), [=] () {
            QString url;
            if (Settings::getInstance()->isTestnet()) {
                url = "https://explorer.testnet.z.cash/tx/" + txid;
            } else {
                url = "https://explorer.zcha.in/transactions/" + txid;
            }
            QDesktopServices::openUrl(QUrl(url));
        });

        menu.exec(md.tblTxids->viewport()->mapToGlobal(pos));
    });

    auto* status = parent->getRPC()->getMigrationStatus();

    md.chkEnabled->setChecked(status->enabled);
    md.lblSaplingAddress->setText(status->saplingAddress);
    md.lblUnMigrated->setText(Settings::getZECDisplayFormat(status->unmigrated));
    md.lblMigrated->setText(Settings::getZECDisplayFormat(status->migrated));

    if (d.exec() == QDialog::Accepted) {
        // Update the migration status if it changed
        if (md.chkEnabled->isChecked() != status->enabled) {
            parent->getRPC()->setMigrationStatus(md.chkEnabled->isChecked());
        }
    }
}


MigrationTxns::MigrationTxns(QTableView *parent, QList<QString> txids)
     : QAbstractTableModel(parent) {
    headers << tr("Migration Txids");
    this->txids = txids;
}


int MigrationTxns::rowCount(const QModelIndex&) const {
    return txids.size();
}

int MigrationTxns::columnCount(const QModelIndex&) const {
    return headers.size();
}

QString MigrationTxns::getTxid(int row) const {
    return txids.at(row);
}

QVariant MigrationTxns::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        switch(index.column()) {
            case 0: return txids.at(index.row());
        }
    }
    return QVariant();
}  


QVariant MigrationTxns::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return headers.at(section);
    }

    return QVariant();
}
