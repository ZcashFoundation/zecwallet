#ifndef RECURRING_H
#define RECURRING_H

#include "precompiled.h"
#include "settings.h"

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
    QString     hashid;

    QString     desc;
    QString     fromAddr;
    QString     toAddr;
    double      amt;
    QString     memo;
    QString     currency;
    Schedule    schedule;
    int         frequency;
    int         numPayments;

    QDateTime   startDate;
    int         completedPayments;

    struct HistoryItem {
        int paymentNumber;
        QDateTime date;
        QString txid;
    };

    QList<HistoryItem> history;
    
    void        updateHash();
    QString     getScheduleDescription();
    QJsonObject toJson();

    static RecurringPaymentInfo fromJson(QJsonObject j);
};

class Recurring
{
public:
    static Recurring* getInstance() { if (!instance) { instance = new Recurring(); } return instance; }

    RecurringPaymentInfo*    getNewRecurringFromTx(QWidget* parent, MainWindow* main, Tx tx, RecurringPaymentInfo* rpi);
    
    QDateTime   getNextPaymentDate(Schedule s);
    void        updateInfoWithTx(RecurringPaymentInfo* r, Tx tx);

    QString     writeableFile();

    void        addRecurringInfo(const RecurringPaymentInfo& rpi);
    void        writeToStorage();
private:
    Recurring() = default;
    QMap<QString, RecurringPaymentInfo> payments;

    static Recurring* instance;
};


#endif // RECURRING_H