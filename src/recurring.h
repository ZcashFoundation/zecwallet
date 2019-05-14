#ifndef RECURRING_H
#define RECURRING_H

#include "precompiled.h"
#include "settings.h"

class MainWindow;
class Recurring;
class RecurringListViewModel;
class RecurringPaymentsListViewModel;

struct Tx;

enum Schedule {
    DAY = 1,
    WEEK,
    MONTH,
    YEAR
};

enum PaymentStatus {
    NOT_STARTED = 0,
    PENDING,
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
    QDateTime   startDate;

    struct PaymentItem {
        int           paymentNumber;
        QDateTime     date;
        QString       txid;
        QString       err;
        PaymentStatus status;
    };

    QList<PaymentItem> payments;

friend class Recurring;    
friend class RecurringListViewModel;
friend class RecurringPaymentsListViewModel;

public:
    RecurringPaymentInfo(int numPayments = 0) {
        // Initialize the payments list.
        for (auto i = 0; i < numPayments; i++) {
            payments.append(
                PaymentItem{i, QDateTime::fromSecsSinceEpoch(0), 
                "", "", PaymentStatus::NOT_STARTED});
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

    static void         showRecurringDialog(MainWindow* parent);
    static QDateTime    getNextPaymentDate(Schedule s, QDateTime start = QDateTime::currentDateTime());

    void        addRecurringInfo(const RecurringPaymentInfo& rpi);
    void        removeRecurringInfo(QString hash);

    void        writeToStorage();
    void        readFromStorage();

    // Worker method that goes through all pending recurring payments to see if any 
    // need to be processed.
    void        processPending(MainWindow* main);
    // If multiple are pending, we need to ask the user
    void        processMultiplePending(RecurringPaymentInfo rpi, MainWindow* main);
    // Execute a particular payment item
    void        executeRecurringPayment(MainWindow *, RecurringPaymentInfo rpi, QList<int> paymentNumber);

    // Execute a Tx
    void        doSendTx(MainWindow* rpc, Tx tx, std::function<void(QString, QString)> cb);

    bool updatePaymentItem(QString hash, int paymentNumber, QString txid, QString err, PaymentStatus status);

    QList<RecurringPaymentInfo> getAsList() { return payments.values(); }
private:
    Recurring() = default;
    QMap<QString, RecurringPaymentInfo> payments;

    static Recurring* instance;
};


// Model for list of configured recurring payments
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

// Model for history of payments
class RecurringPaymentsListViewModel : public QAbstractTableModel {

public:
    RecurringPaymentsListViewModel(QTableView* parent, RecurringPaymentInfo rpi);
    ~RecurringPaymentsListViewModel() = default;

    int      rowCount(const QModelIndex &parent) const;
    int      columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QTableView* parent;
    QStringList headers;
    RecurringPaymentInfo rpi;
};

#endif // RECURRING_H