#ifndef UTILS_H
#define UTILS_H

#include "precompiled.h"

class Utils
{
public:
	static const QString txidStatusMessage;

	static const QString getTokenName();

	static const int 	 updateSpeed = 20 * 1000;	// 20 sec
private: 
	Utils() = delete;
};

#endif // UTILS_H