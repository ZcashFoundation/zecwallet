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

    static RecurringPaymentInfo fromJson(QJsonObject j) {
        RecurringPaymentInfo r;
        r.hashid = j["hash"].toString();
        r.desc = j["desc"].toString();
        r.fromAddr = j["from"].toString();
        r.toAddr = j["to"].toString();
        r.amt = j["amt"].toString().toDouble();
        r.memo = j["memo"].toString();
        r.currency = j["currency"].toString();
        r.schedule = (Schedule)j["schedule"].toInt();
        r.frequency = j["frequency"].toInt();
        r.numPayments = j["numpayments"].toInt();
        r.startDate = QDateTime::fromSecsSinceEpoch(j["startdate"].toString().toLongLong());
        r.completedPayments = j["completed"].toInt();

        r.history = QList<HistoryItem>();
        for (auto h : j["history"].toArray()) {
            HistoryItem item;

            item.paymentNumber = h.toObject()["paymentnumber"].toInt();
            item.date = QDateTime::fromSecsSinceEpoch(h.toObject()["date"].toString().toLongLong());
            item.txid = h.toObject()["txid"].toString();

            r.history.append(item);
        }

        return r;
    }

    void updateHash() {
        auto val = getScheduleDescription() + fromAddr + toAddr;
        hashid = QCryptographicHash::hash(val.toUtf8(), QCryptographicHash::Sha256);
    }

    QJsonObject toJson() {
        QJsonArray historyJson;
        for (auto h : history) {
            historyJson.append(QJsonObject{
                {"paymentnumber", h.paymentNumber},
                {"date", QString::number(h.date.toSecsSinceEpoch())},
                {"txid", h.txid}
            });
        }

        auto j = QJsonObject{
            {"hash", hashid},
            {"desc", desc},
            {"from", fromAddr},
            {"to", toAddr},
            {"amt", Settings::getDecimalString(amt)},
            {"memo", memo},
            {"currency", currency},
            {"schedule", (int)schedule},
            {"frequency", frequency},
            {"numpayments", numPayments},
            {"startdate", QString::number(startDate.toSecsSinceEpoch())},
            {"completed", completedPayments},
            {"history", historyJson}
        };

        return j;
    }

    QString getScheduleDescription() {
        return "Pay " % (currency == "USD" ? Settings::getUSDFormat(amt) : Settings::getZECDisplayFormat(amt))
            % " every " % schedule_desc(schedule) % ", starting " % startDate.toString("yyyy-MMM-dd")
            % ", for " % QString::number(numPayments) % " payments";
    }
};

class Recurring
{
public:
    static Recurring* getInstance() { if (!instance) { instance = new Recurring(); } return instance; }

    RecurringPaymentInfo*    getNewRecurringFromTx(QWidget* parent, MainWindow* main, Tx tx, RecurringPaymentInfo* rpi);
    
    QDateTime    getNextPaymentDate(Schedule s);
    void         updateInfoWithTx(RecurringPaymentInfo* r, Tx tx);
private:
    Recurring() = default;
    static Recurring* instance;
};


#endif // RECURRING_H