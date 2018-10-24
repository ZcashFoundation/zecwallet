#include "utils.h"
#include "settings.h"
#include "mainwindow.h"

const QString Utils::txidStatusMessage = QString("Tx submitted (right click to copy) txid:");

const QString Utils::getTokenName() {
    if (Settings::getInstance()->isTestnet()) {
        return "TAZ";
    } else {
        return "ZEC";
    }
}

const QString Utils::getDonationAddr() {
    if (Settings::getInstance()->isTestnet()) 
        return "tmP1JL19JyJh3jPMUbfovk4W67jB7VJWybu";
    else 
        return "t1KfJJrSuVYmnNLrw7EZHRv1kZY3zdGGLyb";    
}

// Get the dev fee address based on the transaction
const QString Utils::getDevAddr(Tx tx) {
    auto testnetAddrLookup = [=] (const QString& addr) -> QString {
        if (addr.startsWith("ztestsapling")) {
            return "ztestsapling1kdp74adyfsmm9838jaupgfyx3npgw8ut63stjjx757pc248cuc0ymzphqeux60c64qe5qt68ygh";
        } else if (addr.startsWith("zt")) {
            return "ztbGDqgkmXQjheivgeirwEvJLD4SUNqsWCGwxwVg4YpGz1ARR8P2rXaptkT14z3NDKamcxNmQuvmvktyokMs7HkutRNSx1D";
        } else {
            return QString();
        }
    };

    if (Settings::getInstance()->isTestnet()) {
        auto devAddr = testnetAddrLookup(tx.fromAddr);
        if (!devAddr.isEmpty()) {
            return devAddr;
        }

        // t-Addr, find if it is going to a sprout or sapling address
        for (const ToFields& to : tx.toAddrs) {
            devAddr = testnetAddrLookup(to.addr);
            if (!devAddr.isEmpty()) {
                return devAddr;
            }
        }
        
        // If this is a t-Addr -> t-Addr transaction, use the sapling address by default
        return testnetAddrLookup("ztestsapling");
    } else {
        // Mainnet doesn't have a fee yet!
        return QString();
    }
}


double Utils::getMinerFee() {
    return 0.0001;
}
double Utils::getDevFee() {
    if (Settings::getInstance()->isTestnet()) {
        return 0.0001;
    } else {
        return 0;
    }
}
double Utils::getTotalFee() { return getMinerFee() + getDevFee(); }