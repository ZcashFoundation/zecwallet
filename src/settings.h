#ifndef SETTINGS_H
#define SETTINGS_H

#include "precompiled.h"

class Settings
{
public:
    Settings();

    QString getUsernamePassword();
	QString getHost();
	QString getPort();

    void setDefaultPort(int port) {overridePort = QString::number(port);}

    double  fees() { return 0.0001; }
	void    refresh();
private:

    QString host;
	QString port;
    QString username;
    QString password;

    QString overridePort;
};

#endif // SETTINGS_H