#ifndef SETTINGS_H
#define SETTINGS_H


class Settings
{
public:
    Settings();

    QString getUsernamePassword();
	QString getHost();
	QString getPort();

    double  fees() { return 0.0001; }
	void    refresh();
private:

    QString host;
	QString port;
    QString username;
    QString password;    
};

#endif // SETTINGS_H