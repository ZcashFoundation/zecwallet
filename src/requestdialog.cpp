#include "requestdialog.h"
#include "ui_requestdialog.h"

RequestDialog::RequestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RequestDialog)
{
    ui->setupUi(this);
}

RequestDialog::~RequestDialog()
{
    delete ui;
}
