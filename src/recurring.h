#ifndef RECURRING_H
#define RECURRING_H

#include "precompiled.h"

#include "mainwindow.h"

enum Schedule {
    DAY = 1,
    WEEK,
    MONTH,
    YEAR
};

struct RecurringPaymentInfo {
    QString         desc;
    QString         fromAddr;
    QString         toAddr;
    double          amt;
    QString         currency;
    Schedule        schedule;
    int             numPayments;

    long            startBlock;
    int             completedPayments;
};

class Recurring
{
public:
    Recurring();

    static void showEditDialog(QWidget* parent, MainWindow* main, Tx tx);
};

#endif // RECURRING_H