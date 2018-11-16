#include "precompiled.h"

#include "mainwindow.h"
#include "settings.h"

Settings* Settings::instance = nullptr;

Settings* Settings::init() {    
    if (instance == nullptr) 
        instance = new Settings();

    return instance;
}

Settings* Settings::getInstance() {
    return instance;
}

Config Settings::getSettings() {
    // Load from the QT Settings. 
    QSettings s;
    
    auto host        = s.value("connection/host").toString();
    auto port        = s.value("connection/port").toString();
    auto username    = s.value("connection/rpcuser").toString();
    auto password    = s.value("connection/rpcpassword").toString();    

    return Config{host, port, username, password};
}

void Settings::saveSettings(const QString& host, const QString& port, const QString& username, const QString& password) {
    QSettings s;

    s.setValue("connection/host", host);
    s.setValue("connection/port", port);
    s.setValue("connection/rpcuser", username);
    s.setValue("connection/rpcpassword", password);

    s.sync();

    // re-init to load correct settings
    init();
}

void Settings::setUsingZcashConf(QString confLocation) {
    if (!confLocation.isEmpty())
        _confLocation = confLocation;
}

bool Settings::isTestnet() {
    return _isTestnet;
}

void Settings::setTestnet(bool isTestnet) {
    this->_isTestnet = isTestnet;
}

bool Settings::isSaplingAddress(QString addr) {
    return ( isTestnet() && addr.startsWith("ztestsapling")) ||
           (!isTestnet() && addr.startsWith("zs"));
}

bool Settings::isSproutAddress(QString addr) {
    return isZAddress(addr) && !isSaplingAddress(addr);
}

bool Settings::isZAddress(QString addr) {
    return addr.startsWith("z");
}

bool Settings::isSyncing() {
    return _isSyncing;
}

void Settings::setSyncing(bool syncing) {
    this->_isSyncing = syncing;
}

int Settings::getBlockNumber() {
    return this->_blockNumber;
}

void Settings::setBlockNumber(int number) {
    this->_blockNumber = number;
}

bool Settings::isSaplingActive() {
    return  (isTestnet() && getBlockNumber() > 280000) ||
           (!isTestnet() && getBlockNumber() > 419200);
}

double Settings::getZECPrice() { 
    return zecPrice; 
}


bool Settings::getAllowCustomFees() {
    // Load from the QT Settings. 
    return QSettings().value("options/customfees", false).toBool();
}

void Settings::setAllowCustomFees(bool allow) {
    QSettings().setValue("options/customfees", allow);
}

bool Settings::getSaveZtxs() {
    // Load from the QT Settings. 
    return QSettings().value("options/savesenttx", true).toBool();
}

void Settings::setSaveZtxs(bool save) {
    QSettings().setValue("options/savesenttx", save);
}


//=================================
// Static Stuff
//=================================
void Settings::saveRestore(QDialog* d) {
    d->restoreGeometry(QSettings().value(d->objectName() % "geometry").toByteArray());

    QObject::connect(d, &QDialog::finished, [=](auto) {
        QSettings().setValue(d->objectName() % "geometry", d->saveGeometry());
    });
}

QString Settings::getUSDFormat(double bal) {
    if (!Settings::getInstance()->isTestnet() && Settings::getInstance()->getZECPrice() > 0) 
        return "$" + QLocale(QLocale::English).toString(bal * Settings::getInstance()->getZECPrice(), 'f', 2);
    else 
        return QString();
}

QString Settings::getDecimalString(double amt) {
    QString f = QString::number(amt, 'f', 8);
    while (f.contains(".") && (f.right(1) == "0" || f.right(1) == ".")) {
        f = f.left(f.length() - 1);
    }

    return f;
}

QString Settings::getZECDisplayFormat(double bal) {
    // This is idiotic. Why doesn't QString have a way to do this?
    return getDecimalString(bal) % " " % Settings::getTokenName();
}

QString Settings::getZECUSDDisplayFormat(double bal) {
    auto usdFormat = getUSDFormat(bal);
    if (!usdFormat.isEmpty())
        return getZECDisplayFormat(bal) % " (" % getUSDFormat(bal) % ")";
    else
        return getZECDisplayFormat(bal);
}

const QString Settings::txidStatusMessage = QString("Tx submitted (right click to copy) txid:");

const QString Settings::getTokenName() {
    if (Settings::getInstance()->isTestnet()) {
        return "TAZ";
    } else {
        return "ZEC";
    }
}

const QString Settings::getDonationAddr(bool sapling) {
    if (Settings::getInstance()->isTestnet()) 
        if (sapling)
            return "ztestsapling1wn6889vznyu42wzmkakl2effhllhpe4azhu696edg2x6me4kfsnmqwpglaxzs7tmqsq7kudemp5";
        else
            return "ztn6fYKBii4Fp4vbGhkPgrtLU4XjXp4ZBMZgShtopmDGbn1L2JLTYbBp2b7SSkNr9F3rQeNZ9idmoR7s4JCVUZ7iiM5byhF";
    else 
        if (sapling)
            return "zs1gv64eu0v2wx7raxqxlmj354y9ycznwaau9kduljzczxztvs4qcl00kn2sjxtejvrxnkucw5xx9u";
        else
            return "zcEgrceTwvoiFdEvPWcsJHAMrpLsprMF6aRJiQa3fan5ZphyXLPuHghnEPrEPRoEVzUy65GnMVyCTRdkT6BYBepnXh6NBYs";    
}

const QString Settings::getDevSproutAddr() {
    return "ztbGDqgkmXQjheivgeirwEvJLD4SUNqsWCGwxwVg4YpGz1ARR8P2rXaptkT14z3NDKamcxNmQuvmvktyokMs7HkutRNSx1D";
}

// Get the dev fee address based on the transaction
const QString Settings::getDevAddr(Tx) {
    return QString();
}


double Settings::getMinerFee() {
    return 0.0001;
}

double Settings::getZboardAmount() {
    return 0.0001;
}

QString Settings::getZboardAddr() {
    if (Settings::getInstance()->isTestnet()) {
        return getDonationAddr(true);
    }
    else {
        return "zs10m00rvkhfm4f7n23e4sxsx275r7ptnggx39ygl0vy46j9mdll5c97gl6dxgpk0njuptg2mn9w5s";
    }
}

double Settings::getTotalFee() { return getMinerFee(); }

bool Settings::isValidAddress(QString addr) {
    QRegExp zcexp("^z[a-z0-9]{94}$",  Qt::CaseInsensitive);
    QRegExp zsexp("^z[a-z0-9]{77}$",  Qt::CaseInsensitive);
    QRegExp ztsexp("^ztestsapling[a-z0-9]{76}", Qt::CaseInsensitive);
    QRegExp texp("^t[a-z0-9]{34}$", Qt::CaseInsensitive);

    return  zcexp.exactMatch(addr)  || texp.exactMatch(addr) || 
            ztsexp.exactMatch(addr) || zsexp.exactMatch(addr);
}
