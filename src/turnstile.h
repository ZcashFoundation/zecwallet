#ifndef TURNSTILE_H
#define TURNSTILE_H


#include "precompiled.h"

class RPC;

struct TurnstileMigrationItem {
	QString 	fromAddr;
	QString		intTAddr;
	QString		destAddr;
	int			blockNumber;
	double		amount;
	bool 		chaff;
};

class Turnstile
{
public:
	Turnstile(RPC* _rpc);
	~Turnstile();

	void		   planMigration(QString zaddr, QString destAddr);
	QList<double>  splitAmount(double amount, int parts);
	void		   fillAmounts(QList<double>& amounts, double amount, int count);

	void 		   writeMigrationPlan(QList<TurnstileMigrationItem> plan);
	QList<TurnstileMigrationItem> readMigrationPlan();

private:
	QList<int>	   	getBlockNumbers(int start, int end, int count);
	QString		   	writeableFile();
	RPC* 	rpc;	
};

#endif
