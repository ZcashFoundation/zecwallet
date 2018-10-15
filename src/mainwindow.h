#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "precompiled.h"

// Forward declare to break circular dependency.
class RPC;
class Settings;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Settings*           getSettings();

    Ui::MainWindow*     ui;

    QLabel*             statusLabel;
    QLabel*             statusIcon;
    QLabel*             loadingLabel;

private:    
    void setupSendTab();
    void setupTransactionsTab();
    void setupRecieveTab();
    void setupBalancesTab();

	void removeExtraAddresses();
	void setDefaultPayFrom();

	void cancelButton();
	void sendButton();
	void inputComboTextChanged(const QString& text);
	void addAddressSection();
	void maxAmountChecked(int checked);

    QString doSendTxValidations(QString fromAddr, QList<QPair<QString, double>> toAddrs);

    void donate();

    RPC*                rpc;
    Settings*           settings;

    QMovie*             loadingMovie;
};

#endif // MAINWINDOW_H
