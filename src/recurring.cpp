#include "recurring.h"
#include "ui_recurringdialog.h"

RecurringDialog::RecurringDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RecurringDialog)
{
    ui->setupUi(this);
}

RecurringDialog::~RecurringDialog()
{
    delete ui;
}
