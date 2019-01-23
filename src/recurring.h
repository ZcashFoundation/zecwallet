#ifndef RECURRING_H
#define RECURRING_H

#include <QDialog>

namespace Ui {
class RecurringDialog;
}

class RecurringDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RecurringDialog(QWidget *parent = nullptr);
    ~RecurringDialog();

private:
    Ui::RecurringDialog *ui;
};

#endif // RECURRING_H
