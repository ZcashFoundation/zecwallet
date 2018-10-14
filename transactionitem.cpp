#include "transactionitem.h"

TransactionItem::TransactionItem(const QString& type, const QString& datetime, const QString& address, const QString& txid,
                                  double amount, int confirmations)
{
    this->type          = type;
    this->datetime      = datetime;
    this->address       = address;
    this->txid          = txid;
    this->amount        = amount;
    this->confirmations = confirmations;
}
