#include "turnstile.h"

#include "utils.h"

Turnstile::Turnstile() {
}


Turnstile::~Turnstile() {
}

QList<double> Turnstile::splitAmount(double amount, int parts) {
	QList<double> amounts;
	fillAmounts(amounts, amount, parts);

	// Ensure they all add up!
	double sumofparts = 0;
	for (auto a : amounts) {
		sumofparts += a;
	}

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

		// Reduce the Dev Tx fee from the amount
		actual = actual - 0.0001; //Utils::getDevFee();

		// Calculate the chaff. 
		double chaff  = amount - actual;

		amounts.push_back(actual);
		if (chaff > 0.00000001)	// zcash is split down to 8 decimal places
			amounts.push_back(chaff);
		return;
	}

	// Get a random amount off the amount (between half and full) and call recursively.
	auto curAmount = std::rand() % (int)std::floor(amount);

	// Try to round it off
	if (curAmount > 1000) {
		curAmount = std::floor(curAmount / 100) * 100;
	} else if (curAmount > 100) {
		curAmount = std::floor(curAmount / 10) * 10;
	}

	if (curAmount > 0)
		amounts.push_back(curAmount);

	fillAmounts(amounts, amount - curAmount, count - 1);
}