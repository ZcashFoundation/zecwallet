#ifndef TRANSACTIONITEM_H
#define TRANSACTIONITEM_H

#include "precompiled.h"

class TransactionItem
{
public:
    TransactionItem(const QString& type, const QString& datetime, const QString& address, const QString& txid,
                    double amount, int confirmations);

    QString type;
    QString datetime;
    QString address;
    QString txid;
    double  amount;
    int     confirmations;
};

#endif // TRANSACTIONITEM_H
