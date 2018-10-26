#ifndef UNSPENTOUTPUT_H
#define UNSPENTOUTPUT_H

#include "precompiled.h"

struct UnspentOutput {
    QString address;
    QString txid;
    QString amount;    
    int     confirmations;
    bool    spendable;
};


#endif // UNSPENTOUTPUT_H
