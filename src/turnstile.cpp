#include "precompiled.h"
#include "turnstile.h"


Turnstile::Turnstile() {
}


Turnstile::~Turnstile() {
}

QList<QString> Turnstile::splitAmount(double amount, int parts) {
	QList<QString> amounts;
	fillAmounts(amounts, amount, parts);

	// Ensure they all add up!
	double sumofparts = 0;
	for (auto a : amounts) {
		sumofparts += a.toDouble();
	}

	Q_ASSERT(sumofparts == amount);
	return amounts;
}

void Turnstile::fillAmounts(QList<QString>& amounts, double amount, int count) {
	if (count == 1 || amount < 1) {
		amounts.push_back(QString::number(amount, 'g', 8));
		return;
	}

	// Get a random amount off the amount and call recursively.
	auto curAmount = std::rand() % (int)std::floor(amount);
	amounts.push_back(QString::number(curAmount, 'g', 8));

	fillAmounts(amounts, amount - curAmount, count - 1);
}