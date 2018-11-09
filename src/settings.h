#ifndef SETTINGS_H
#define SETTINGS_H

#include "precompiled.h"

struct Config {
    QString host;
    QString port;
    QString rpcuser;
    QString rpcpassword;
};

class Settings
{
public:
    static Settings* init();
    static Settings* getInstance();

    static void saveRestore(QDialog* d);

    Config getSettings();
    void   saveSettings(const QString& host, const QString& port, const QString& username, const QString& password);

    bool isTestnet();
    void setTestnet(bool isTestnet);

    bool isSaplingAddress(QString addr);
    bool isSproutAddress(QString addr);
    bool isZAddress(QString addr);

    bool isSyncing();
    void setSyncing(bool syncing);

    QString getExecName() { return _executable; }
    void    setExecName(QString name) { _executable = name; }

    void setUseEmbedded(bool r) { _useEmbedded = r; }
    bool useEmbedded() { return _useEmbedded; }

    int  getBlockNumber();
    void setBlockNumber(int number);

    bool getSaveZtxs();
    void setSaveZtxs(bool save);

    bool isSaplingActive();

    void  setUsingZcashConf(QString confLocation);
    const QString& getZcashdConfLocation() { return _confLocation; }

    void    setZECPrice(double p) { zecPrice = p; }
    double  getZECPrice();
        
    QString getUSDFormat          (double bal);
    QString getZECDisplayFormat   (double bal);
    QString getZECUSDDisplayFormat(double bal);

private:
    // This class can only be accessed through Settings::getInstance()
    Settings() = default;
    ~Settings();

    static Settings* instance;

    QString _confLocation;
    QString _executable;
    bool    _isTestnet        = false;
    bool    _isSyncing        = false;
    int     _blockNumber      = 0;
    bool    _useEmbedded      = false;

    double zecPrice = 0.0;
};

#endif // SETTINGS_H