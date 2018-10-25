#include "turnstile.h"
#include "mainwindow.h"
#include "unspentoutput.h"
#include "rpc.h"
#include "utils.h"
#include "settings.h"

#include "precompiled.h"

using json = nlohmann::json;

Turnstile::Turnstile(RPC* _rpc) {
	this->rpc = _rpc;
}

Turnstile::~Turnstile() {
}

void printPlan(QList<TurnstileMigrationItem> plan) {
	for (auto item : plan) {
		qDebug() << item.fromAddr << item.intTAddr 
					<< item.destAddr << item.amount << item.blockNumber << item.status;
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
	qDebug() << QString("Writing plan");
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

void Turnstile::planMigration(QString zaddr, QString destAddr) {
	// First, get the balance and split up the amounts
	auto bal = rpc->getAllBalances()->value(zaddr);
	auto splits = splitAmount(bal, 5);

	// Then, generate an intermediate t-Address for each part using getBatchRPC
	rpc->getBatchRPC<double>(splits,
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
			auto blockNumbers = getBlockNumbers(curBlock, curBlock + 3, splits.size());

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
			migItems[0].blockNumber = curBlock;

			std::sort(migItems.begin(), migItems.end(), [&] (auto a, auto b) {
				return a.blockNumber < b.blockNumber;
			});		

			writeMigrationPlan(migItems);
			auto readPlan = readMigrationPlan();
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

QList<double> Turnstile::splitAmount(double amount, int parts) {
	QList<double> amounts;
	// Need at least 0.0004 ZEC for this
	if (amount < 0.0004)
		return amounts;
	
	fillAmounts(amounts, amount, parts);

	// Ensure they all add up!
	double sumofparts = 0;
	for (auto a : amounts) {
		sumofparts += a;
	}
	
	// Add the Tx fees
	sumofparts += amounts.size() * Utils::getMinerFee();

	Q_ASSERT(sumofparts == amount);
	return amounts;
}

void Turnstile::fillAmounts(QList<double>& amounts, double amount, int count) {
	if (count == 1 || amount < 1) {
		// Split the chaff.
		// Chaff is all amounts lesser than 0.0001 ZEC. The chaff will be added to the 
		// dev fee, and is done so to protect privacy.

		// Get the rounded value to 4 decimal places (approx $0.01)
		double actual = std::floor(amount * 10000) / 10000;

		// Also account for the fees needed to send all these transactions
		actual = actual - (Utils::getMinerFee() * (amounts.size() + 1));

		amounts.push_back(actual);
		return;
	}

	// Get a random amount off the amount (between half and full) and call recursively.
	auto curAmount = std::rand() % (int)std::floor(amount);

	// Try to round it off
	// TODO: Do this via log
	if (curAmount > 1000) {
		curAmount = std::floor(curAmount / 100) * 100;
	} else if (curAmount > 100) {
		curAmount = std::floor(curAmount / 10) * 10;
	}

	if (curAmount > 0)
		amounts.push_back(curAmount);

	fillAmounts(amounts, amount - curAmount, count - 1);
}

void Turnstile::executeMigrationStep() {
	auto plan = readMigrationPlan();

	qDebug() << QString("Executing step");
	printPlan(plan);

	// Get to the next unexecuted step
	auto fnIsEligibleItem = [&] (auto item) {
		return 	item.status == TurnstileMigrationItemStatus::NotStarted || 
				item.status == TurnstileMigrationItemStatus::SentToT;
	};

	// Fn to find if there are any unconfirmed funds for this address.
	auto fnHasUnconfirmed = [=] (QString addr) {
		auto utxoset = rpc->getUTXOs();
		return std::find_if(utxoset->begin(), utxoset->end(), [=] (auto utxo) {
					return utxo.address == addr && utxo.confirmations == 0;
				}) != utxoset->end();
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
		auto balance = rpc->getAllBalances()->value(nextStep->fromAddr);
		if (nextStep->amount > balance) {
			qDebug() << "Not enough balance!";
			nextStep->status = TurnstileMigrationItemStatus::NotEnoughBalance;
			writeMigrationPlan(plan);
			return;
		}

		auto to = ToFields{ nextStep->intTAddr, nextStep->amount, "", "" };

		// If this is the last step, then send the remaining amount instead of the actual amount.
		if (lastStep) {
			auto remainingAmount = balance - Utils::getMinerFee();
			if (remainingAmount > 0) {
				to.amount = remainingAmount;
			}
		}

		// Create the Tx
		auto tx = Tx{ nextStep->fromAddr, { to }, Utils::getMinerFee() };

		// And send it
		doSendTx(tx, [=] () {
			// Update status and write plan to disk
			nextStep->status = TurnstileMigrationItemStatus::SentToT;
			writeMigrationPlan(plan);
		});

	} else if (nextStep->status == TurnstileMigrationItemStatus::SentToT) {
		// First thing to do is check to see if the funds are confirmed. 
		if (fnHasUnconfirmed(nextStep->intTAddr)) {
			qDebug() << QString("unconfirmed, waiting");
			return;
		}

		// Send it to the final destination address.
		auto bal = rpc->getAllBalances()->value(nextStep->intTAddr);
		auto sendAmt = bal - Utils::getMinerFee();

		if (sendAmt < 0) {
			qDebug() << "Not enough balance!";
			nextStep->status = TurnstileMigrationItemStatus::NotEnoughBalance;
			writeMigrationPlan(plan);
			return;
		}
		
		QList<ToFields> to = { ToFields{ nextStep->destAddr, sendAmt, "", "" } };

		// Create the Tx
		auto tx = Tx{ nextStep->intTAddr, to, Utils::getMinerFee() };

		// And send it
		doSendTx(tx, [=] () {
			// Update status and write plan to disk
			nextStep->status = TurnstileMigrationItemStatus::SentToZS;
			writeMigrationPlan(plan);
		});
	}
}

void Turnstile::doSendTx(Tx tx, std::function<void(void)> cb) {
	json params = json::array();
	rpc->fillTxJsonParams(params, tx);
	std::cout << std::setw(2) << params << std::endl;
	rpc->sendZTransaction(params, [=] (const json& reply) {
		QString opid = QString::fromStdString(reply.get<json::string_t>());
		qDebug() << opid;
		//ui->statusBar->showMessage("Computing Tx: " % opid);

		// And then start monitoring the transaction
		rpc->addNewTxToWatch(tx, opid);

		cb();
	});
}