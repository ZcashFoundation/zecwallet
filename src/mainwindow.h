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

    RPC*                rpc;
    Settings*           settings;
};

#endif // MAINWINDOW_H
