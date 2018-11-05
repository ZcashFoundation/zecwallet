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
    static const QString getDevSproutAddr();
    static const QString getDevAddr(Tx tx);
    static const QString getDonationAddr(bool sapling);

    static double  getMinerFee();
    static double  getZboardAmount();
    static QString getZboardAddr();
    static double  getDevFee();
    static double  getTotalFee();

    static bool    isValidAddress(QString addr);

    static const int     updateSpeed         = 20 * 1000;        // 20 sec
    static const int     quickUpdateSpeed    = 5  * 1000;        // 5 sec
    static const int     priceRefreshSpeed   = 60 * 60 * 1000;    // 1 hr
private:     
    Utils() = delete;
};

#endif // UTILS_H