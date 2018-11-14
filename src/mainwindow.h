#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "precompiled.h"

// Forward declare to break circular dependency.
class RPC;
class Settings;

using json = nlohmann::json;

// Struct used to hold destination info when sending a Tx. 
struct ToFields {
    QString addr;
    double  amount;
    QString txtMemo;
    QString encodedMemo;
};

// Struct used to represent a Transaction. 
struct Tx {
    QString         fromAddr;
    QList<ToFields> toAddrs;
    double          fee;
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

    void updateLabelsAutoComplete();

    Ui::MainWindow*     ui;

    QLabel*             statusLabel;
    QLabel*             statusIcon;
    QLabel*             loadingLabel;

private:    
    void closeEvent(QCloseEvent* event);

    void setupSendTab();
    void setupTransactionsTab();
    void setupRecieveTab();
    void setupBalancesTab();
    void setupZcashdTab();

    void setupTurnstileDialog();
    void setupSettingsModal();
    void setupStatusBar();

    void removeExtraAddresses();
    void setDefaultPayFrom();

    Tx   createTxFromSendPage();
    bool confirmTx(Tx tx);

    void turnstileDoMigration(QString fromAddr = "");
    void turnstileProgress();

    void cancelButton();
    void sendButton();
    void inputComboTextChanged(const QString& text);
    void addAddressSection();
    void maxAmountChecked(int checked);

    void addressChanged(int number, const QString& text);
    void amountChanged (int number, const QString& text);

    void addNewZaddr(bool sapling);
    std::function<void(bool)> addZAddrsToComboList(bool sapling);

    void memoButtonClicked(int number);
    void setMemoEnabled(int number, bool enabled);

    QString doSendTxValidations(Tx tx);

    void donate();
    void addressBook();
    void postToZBoard();
    void importPrivKey();
    void exportAllKeys();
    void doImport(QList<QString>* keys);

    void restoreSavedStates();

    QString addressFromAddressField(const QString& lblAddr) { return lblAddr.trimmed().split("/").last(); }

    RPC*         rpc  = nullptr;
    QCompleter*  labelCompleter = nullptr;

    QMovie*      loadingMovie;
};

#endif // MAINWINDOW_H
