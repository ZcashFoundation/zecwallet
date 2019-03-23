#ifndef REQUESTDIALOG_H
#define REQUESTDIALOG_H

#include <QDialog>
#include "mainwindow.h"
#include "ui_requestdialog.h"

namespace Ui {
class RequestDialog;
}

class RequestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RequestDialog(QWidget *parent = nullptr);
    ~RequestDialog();

    static void showRequestZcash(MainWindow* main);
    static void showPaymentConfirmation(MainWindow* main, QString paymentURI);
    static void setupDialog(MainWindow* main, QDialog* d, Ui_RequestDialog* req);
private:
    Ui::RequestDialog *ui;
};

#endif // REQUESTDIALOG_H
