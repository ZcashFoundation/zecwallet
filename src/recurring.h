#ifndef RECURRING_H
#define RECURRING_H

#include "precompiled.h"

class MainWindow;
struct Tx;

enum Schedule {
    DAY = 1,
    WEEK,
    MONTH,
    YEAR
};

QString schedule_desc(Schedule s);

struct RecurringPaymentInfo {
    QString         desc;
    QString         fromAddr;
    QString         toAddr;
    double          amt;
    QString         memo;
    QString         currency;
    Schedule        schedule;
    int             numPayments;

    QDateTime       startDate;
    int             completedPayments;

    QString getScheduleDescription() {
        return "Every " % schedule_desc(schedule) % ", starting " % startDate.toString("yyyy-MMM-dd")
            % ", for " % QString::number(numPayments) % " payments";
    }
};

class Recurring
{
public:
    Recurring() = default;

    static RecurringPaymentInfo*    getNewRecurringFromTx(QWidget* parent, MainWindow* main, Tx tx, RecurringPaymentInfo* rpi);
    
    static QDateTime    getNextPaymentDate(Schedule s);
    static void         updateInfoWithTx(RecurringPaymentInfo* r, Tx tx);
};


#endif // RECURRING_H