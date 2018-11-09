#ifndef ADDRESSBOOK_H
#define ADDRESSBOOK_H

#include "precompiled.h"

class MainWindow;

class AddressBookModel : public QAbstractTableModel {

public:
    AddressBookModel(QTableView* parent);
    ~AddressBookModel();

    void addNewLabel(QString label, QString addr);
    void removeItemAt(int row);
    QPair<QString, QString> itemAt(int row);

    int      rowCount(const QModelIndex &parent) const;
    int      columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    void loadData();
    void saveData();

    QTableView* parent;
    QList<QPair<QString, QString>> labels;
    QStringList headers;    
};

class AddressBook {
public:    
    static void open(MainWindow* parent, QLineEdit* target = nullptr);

    static QList<QPair<QString, QString>> readFromStorage();
    static void writeToStorage(QList<QPair<QString, QString>> labels);

    static QString writeableFile();
};

#endif // ADDRESSBOOK_H