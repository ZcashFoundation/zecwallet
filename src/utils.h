#ifndef UTILS_H
#define UTILS_H

#include "precompiled.h"

struct ToFields;
struct Tx;

class Utils
{
public:
	static const QString txidStatusMessage;

	static const QString getTokenName();
	static const QString getDevAddr(Tx tx);

	static double  getMinerFee();
	static double  getDevFee();
	static double  getTotalFee();

	static const int 	updateSpeed 		= 20 * 1000;		// 20 sec
	static const int 	quickUpdateSpeed	= 5  * 1000;		// 5 sec
	static const int 	priceRefreshSpeed	= 60 * 60 * 1000;	// 1 hr
private: 	
	Utils() = delete;
};

#endif // UTILS_H