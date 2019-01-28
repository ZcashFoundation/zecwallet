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
        int         paymentNumber;
        QDateTime   date;
        QString     txid;
        QString     status;
    };

    QList<HistoryItem> history;
    
    void        updateHash();
    QString     getScheduleDescription();
    QJsonObject toJson();


    QString getAmountPretty();

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

    QList<RecurringPaymentInfo> getAsList() { return payments.values(); }
private:
    Recurring() = default;
    QMap<QString, RecurringPaymentInfo> payments;

    static Recurring* instance;
};


class RecurringListViewModel : public QAbstractTableModel {

public:
    RecurringListViewModel(QTableView* parent);
    ~RecurringListViewModel() = default;

    int      rowCount(const QModelIndex &parent) const;
    int      columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QTableView* parent;
    QStringList headers;
};

#endif // RECURRING_H