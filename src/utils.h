#ifndef UTILS_H
#define UTILS_H

#include "precompiled.h"

struct ToFields;

class Utils
{
public:
	static const QString txidStatusMessage;

	static const QString getTokenName();
	static const QString getDevAddr(const QString& fromAddr, const QList<ToFields>& toAddrs);

	static double  getMinerFee();
	static double  getDevFee();
	static double  getTotalFee();

	static const int 	 updateSpeed = 20 * 1000;	// 20 sec
private: 	
	Utils() = delete;
};

#endif // UTILS_H