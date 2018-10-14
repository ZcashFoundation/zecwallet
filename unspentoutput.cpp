#include "unspentoutput.h"

UnspentOutput::UnspentOutput(QString address, QString txid, QString amount, int confirmations)
{
    this->address       = address;
    this->txid          = txid;
    this->amount        = amount;
    this->confirmations = confirmations;
}
