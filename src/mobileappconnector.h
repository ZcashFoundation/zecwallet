#ifndef MOBILEAPPCONNECTOR_H
#define MOBILEAPPCONNECTOR_H

#include <QDialog>

namespace Ui {
class MobileAppConnector;
}

class MobileAppConnector : public QDialog
{
    Q_OBJECT

public:
    explicit MobileAppConnector(QWidget *parent = nullptr);
    ~MobileAppConnector();

private:
    Ui::MobileAppConnector *ui;
};

#endif // MOBILEAPPCONNECTOR_H
