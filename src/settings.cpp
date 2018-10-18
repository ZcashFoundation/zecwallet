#include "precompiled.h"

#include "settings.h"

Settings* Settings::instance = nullptr;

Settings* Settings::init() {	
	if (instance != nullptr) return instance;
	
	instance = new Settings();

	// Load from settings first, because if they are redefined in the zcash.conf file, 
	// we'll overwrite them. 
	instance->loadFromSettings();    
#ifdef Q_OS_LINUX
	// Overwrite if any are defined in the zcash.conf
	instance->loadFromFile();
#endif
	return instance;
}

Settings* Settings::getInstance() {
	return instance;
}

QString Settings::getHost() {
	if (host.isNull() || host == "") return "127.0.0.1";
	return host;
}

QString Settings::getPort() {
	// If the override port is set, we'll always return it
	if (!overridePort.isEmpty()) return overridePort;
	
	if (port.isNull() || port == "") return "8232";
	return port;
}


QString Settings::getUsernamePassword() {
    return username % ":" % password;
}

void Settings::loadFromSettings() {
	// First step is to try and load from the QT Settings. These are loaded first, because
	// they could be overridden by whats in the zcash.conf, which will take precedence. 
	QSettings s;
	
	host		= s.value("connection/host",		"127.0.0.1"	).toString();
	port		= s.value("connection/port",		"8232"		).toString();
	username	= s.value("connection/rpcuser",		""			).toString();
	password	= s.value("connection/rpcpassword", ""			).toString();	
}

void Settings::loadFromFile() {
	// Nothing in QT Settings, so try to read from file. 
	QString zcashdconf = QStandardPaths::locate(QStandardPaths::HomeLocation, ".zcash/zcash.conf");
	if (zcashdconf.isNull()) {
		// No zcash file, just return with nothing
		return;
	}

	QFile file(zcashdconf);
	if (!file.open(QIODevice::ReadOnly)) {
		qDebug() << file.errorString();
		return;
	}

	QTextStream in(&file);

	while (!in.atEnd()) {
		QString line = in.readLine();
		QStringList fields = line.split("=");

		if (fields[0].trimmed().toLower() == "rpcuser") {
			fields.removeFirst();
			username = fields.join("").trimmed();
		}
		if (fields[0].trimmed().toLower() == "rpcpassword") {
			fields.removeFirst();
			password = fields.join("").trimmed();
		}
		if (fields[0].trimmed().toLower() == "rpcport") {
			overridePort = fields[1].trimmed();
		}
		if (fields[0].trimmed().toLower() == "testnet" &&
			fields[1].trimmed() == "1" &&
			overridePort.isEmpty()) {
				overridePort = "18232";
		}
	}

	file.close();

}

bool Settings::isTestnet() {
	return _isTestnet;
}

void Settings::setTestnet(bool isTestnet) {
	this->_isTestnet = isTestnet;
}

bool Settings::isSyncing() {
	return _isSyncing;
}

void Settings::setSyncing(bool syncing) {
	this->_isSyncing = syncing;
}