#ifndef TURNSTILE_H
#define TURNSTILE_H

#include "precompiled.h"

class RPC;
struct Tx;

struct TurnstileMigrationItem {
	QString 	fromAddr;
	QString		intTAddr;
	QString		destAddr;
	int			blockNumber;
	double		amount;
	bool 		chaff;
	int 		status;
};

enum TurnstileMigrationItemStatus {
	NotStarted = 0,
	SentToT,
	SentToZS,
	NotEnoughBalance,
	UnknownError
};

class Turnstile
{
public:
	Turnstile(RPC* _rpc);
	~Turnstile();

	void		   	planMigration(QString zaddr, QString destAddr);
	QList<double>  	splitAmount(double amount, int parts);
	void		   	fillAmounts(QList<double>& amounts, double amount, int count);

	QList<TurnstileMigrationItem> readMigrationPlan();
	void 		   	writeMigrationPlan(QList<TurnstileMigrationItem> plan);
	
	void 			executeMigrationStep();

private:
	QList<int>	   	getBlockNumbers(int start, int end, int count);
	QString		   	writeableFile();

	void 			doSendTx(Tx tx, std::function<void(void)> cb);

	RPC* 	rpc;	
};

#endif
