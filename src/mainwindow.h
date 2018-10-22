#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "precompiled.h"

// Forward declare to break circular dependency.
class RPC;
class Settings;

// Struct used to hold destination info when sending a Tx. 
struct ToFields {
    QString addr;
    double  amount;
    QString txtMemo;
    QString encodedMemo;
};

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Ui::MainWindow*     ui;

    QLabel*             statusLabel;
    QLabel*             statusIcon;
    QLabel*             loadingLabel;

private:    
    void setupSendTab();
    void setupTransactionsTab();
    void setupRecieveTab();
    void setupBalancesTab();

	void setupSettingsModal();
	void setupStatusBar();

	void removeExtraAddresses();
	void setDefaultPayFrom();

	void cancelButton();
	void sendButton();
	void inputComboTextChanged(const QString& text);
	void addAddressSection();
	void maxAmountChecked(int checked);

    void addressChanged(int number, const QString& text);
    void amountChanged (int numer, const QString& text);

    void addNewZaddr(bool sapling);
    std::function<void(bool)> addZAddrsToComboList(bool sapling);

    void memoButtonClicked(int number);
    void setMemoEnabled(int number, bool enabled);

    QString doSendTxValidations(QString fromAddr, QList<ToFields> toAddrs);

    void donate();
    void importPrivKeys();

    RPC*                rpc;
    Settings*           settings;

    QMovie*             loadingMovie;
};

#endif // MAINWINDOW_H
