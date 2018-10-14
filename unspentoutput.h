#ifndef UNSPENTOUTPUT_H
#define UNSPENTOUTPUT_H

#include "precompiled.h"

class UnspentOutput
{
public:
    UnspentOutput(QString address, QString txid, QString amount, int confirmations);

    QString address;
    QString txid;
    QString amount;    
    int     confirmations;
};


#endif // UNSPENTOUTPUT_H
