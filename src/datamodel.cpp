#include "datamodel.h"

DataModel::DataModel() {
    lock = new QReadWriteLock();

    // Write lock because we're creating everything
    QWriteLocker locker(lock);

    utxos = new QList<UnspentOutput>();
    balances = new QMap<QString, double>();
    usedAddresses = new QMap<QString, bool>();
    zaddresses = new QList<QString>();
    taddresses = new QList<QString>();
}

DataModel::~DataModel() {
    delete lock;

    delete utxos;
    delete balances;
    delete usedAddresses;
    delete zaddresses;
    delete taddresses;
}

void DataModel::replaceZaddresses(QList<QString>* newZ) {
    QWriteLocker locker(lock);
    Q_ASSERT(newZ);

    delete zaddresses;
    zaddresses = newZ;
}


void DataModel::replaceTaddresses(QList<QString>* newT) {
    QWriteLocker locker(lock);
    Q_ASSERT(newT);

    delete taddresses;
    taddresses = newT;
}

void DataModel::replaceBalances(QMap<QString, double>* newBalances) {
    QWriteLocker locker(lock);
    Q_ASSERT(newBalances);

    delete balances;
    balances = newBalances;
}


void DataModel::replaceUTXOs(QList<UnspentOutput>* newutxos) {
    QWriteLocker locker(lock);
    Q_ASSERT(newutxos);

    delete utxos;
    utxos = newutxos;
}

void DataModel::markAddressUsed(QString address) {
    QWriteLocker locker(lock);

    usedAddresses->insert(address, true);
}