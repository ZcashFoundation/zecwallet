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

    double  fees() { return 0.0001; }
	bool    loadFromSettings();
    bool    loadFromFile();

    bool isTestnet();
    void setTestnet(bool isTestnet);

	bool isSyncing();
	void setSyncing(bool syncing);

	const QString& getZcashdConfLocation() { return confLocation; }

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

    bool _isTestnet = false;
	bool _isSyncing = false;
};

#endif // SETTINGS_H