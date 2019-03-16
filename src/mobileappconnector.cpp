#include "mobileappconnector.h"
#include "ui_mobileappconnector.h"

MobileAppConnector::MobileAppConnector(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MobileAppConnector)
{
    ui->setupUi(this);
}

MobileAppConnector::~MobileAppConnector()
{
    delete ui;
}
