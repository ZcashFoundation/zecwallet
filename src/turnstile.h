#ifndef TURNSTILE_H
#define TURNSTILE_H

#include "precompiled.h"

class Turnstile
{
public:
	Turnstile();
	~Turnstile();

	QList<double>  splitAmount(double amount, int parts);
	void		   fillAmounts(QList<double>& amounts, double amount, int count);
};

#endif