#ifndef SETTINGS_H
#define SETTINGS_H

#include "precompiled.h"

class Settings
{
public:
    static Settings* init();
    static Settings* getInstance();

    QString getUsernamePassword();
	QString getHost();
	QString getPort();

    void setDefaultPort(int port) {overridePort = QString::number(port);}

    double  fees() { return 0.0001; }
	void    loadFromSettings();
    void    loadFromFile();
private:
    // This class can only be accessed through Settings::getInstance()
    Settings() = default;

    static Settings* instance;

    QString host;
	QString port;
    QString username;
    QString password;

    QString overridePort;
};

#endif // SETTINGS_H