#include "precompiled.h"

#include "settings.h"

Settings::Settings()
{	
	refresh();    
}

QString Settings::getHost() {
	if (host.isNull() || host == "") return "127.0.0.1";
	return host;
}

QString Settings::getPort() {
	if (port.isNull() || port == "") return "8232";
	return port;
}


QString Settings::getUsernamePassword() {
    return username % ":" % password;
}

void Settings::refresh() {
	// First step is to try and load from the QT Settings
	QSettings s;
	
	host		= s.value("connection/host",		"127.0.0.1"	).toString();
	port		= s.value("connection/port",		"8232"		).toString();
	username	= s.value("connection/rpcuser",		""			).toString();
	password	= s.value("connection/rpcpassword", ""			).toString();
	
	if (username == "") {
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
				username = fields[1].trimmed();
			}
			if (fields[0].trimmed().toLower() == "rpcpassword") {
				password = fields[1].trimmed();
			}
		}

		file.close();
	}
}