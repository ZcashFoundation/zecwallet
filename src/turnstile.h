#pragma once
class Turnstile
{
public:
	Turnstile();
	~Turnstile();

	QList<QString> Turnstile::splitAmount(double amount, int parts);
	void		   fillAmounts(QList<QString>& amounts, double amount, int count);
};

