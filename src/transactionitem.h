#ifndef TRANSACTIONITEM_H
#define TRANSACTIONITEM_H

#include "precompiled.h"

struct TransactionItem {
    QString         type;
    unsigned long   datetime;
    QString         address;
    QString         txid;
    double          amount;
    unsigned long   confirmations;
};

#endif // TRANSACTIONITEM_H
