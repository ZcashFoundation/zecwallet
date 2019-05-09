#ifndef RECURRING_H
#define RECURRING_H

#include "precompiled.h"
#include "settings.h"

class MainWindow;
class Recurring;
class RecurringListViewModel;

struct Tx;

enum Schedule {
    DAY = 1,
    WEEK,
    MONTH,
    YEAR
};

enum PaymentStatus {
    NOT_STARTED = 0,
    SKIPPED,
    COMPLETED,
    ERROR,
    UNKNOWN
};

QString schedule_desc(Schedule s);

class RecurringPaymentInfo {
private:
    QString     desc;
    QString     fromAddr;
    QString     toAddr;
    double      amt;
    QString     memo;
    QString     currency;
    Schedule    schedule;
    int         frequency;
    QDateTime   startDate;

    struct PaymentItem {
        int           paymentNumber;
        QDateTime     date;
        QString       txid;
        PaymentStatus status;
    };

    QList<PaymentItem> payments;

friend class Recurring;    
friend class RecurringListViewModel;

public:
    RecurringPaymentInfo(int numPayments = 0) {
        // Initialize the payments list.
        for (auto i = 0; i < numPayments; i++) {
            payments.append(
                PaymentItem{i, QDateTime::fromSecsSinceEpoch(0), 
                "", PaymentStatus::NOT_STARTED});
        }
    }

    QString     getScheduleDescription() const;
    QJsonObject toJson();

    QString   getAmountPretty() const;
    QString   getHash() const;
    int       getNumPendingPayments() const;
    QDateTime getNextPayment() const;

    static RecurringPaymentInfo fromJson(QJsonObject j);
};

class Recurring
{
public:
    static Recurring* getInstance();

    RecurringPaymentInfo*    getNewRecurringFromTx(QWidget* parent, MainWindow* main, Tx tx, RecurringPaymentInfo* rpi);   
 
    void        updateInfoWithTx(RecurringPaymentInfo* r, Tx tx);
    QString     writeableFile();
    void        readFromFile();

    static void         showRecurringDialog();
    static QDateTime    getNextPaymentDate(Schedule s, QDateTime start = QDateTime::currentDateTime());

    void        addRecurringInfo(const RecurringPaymentInfo& rpi);
    void        writeToStorage();

    bool updatePaymentItem(QString hash, int paymentNumber, QString txid, PaymentStatus status);

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