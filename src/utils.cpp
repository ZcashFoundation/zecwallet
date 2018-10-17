#include "utils.h"
#include "settings.h"

const QString Utils::txidStatusMessage = QString("Tx submitted (right click to copy) txid:");

const QString Utils::getTokenName() {
    if (Settings::getInstance()->isTestnet()) {
        return "TAZ";
    } else {
        return "ZEC";
    }
}