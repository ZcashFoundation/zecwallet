#ifndef REQUESTDIALOG_H
#define REQUESTDIALOG_H

#include <QDialog>

namespace Ui {
class RequestDialog;
}

class RequestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RequestDialog(QWidget *parent = nullptr);
    ~RequestDialog();

private:
    Ui::RequestDialog *ui;
};

#endif // REQUESTDIALOG_H
