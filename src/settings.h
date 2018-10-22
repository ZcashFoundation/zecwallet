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

    QString getUsernamePassword();
	QString getHost();
	QString getPort();

	bool    loadFromSettings();
    bool    loadFromFile();

	void saveSettings(const QString& host, const QString& port, const QString& username, const QString& password);

    bool isTestnet();
    void setTestnet(bool isTestnet);

	bool isSaplingAddress(QString addr);

	bool isSyncing();
	void setSyncing(bool syncing);

	int  getBlockNumber();
	void setBlockNumber(int number);

	bool isSaplingActive();

	const QString& getZcashdConfLocation() { return confLocation; }

	void   	setZECPrice(double p) 		{ zecPrice = p; }
	double 	getZECPrice();

	QString getUSDFormat			(double bal);
	QString getZECDisplayFormat		(double bal);
	QString getZECUSDDisplayFormat	(double bal);

private:
    // This class can only be accessed through Settings::getInstance()
    Settings() = default;
	~Settings();

    static Settings* instance;

	Config*		currentConfig;

	Config*		defaults	= nullptr;
	Config*		zcashconf	= nullptr;
	Config*     uisettings	= nullptr;

	QString     confLocation;

    bool _isTestnet   = false;
	bool _isSyncing   = false;
	int  _blockNumber = 0;

	double zecPrice = 0.0;
};

#endif // SETTINGS_H