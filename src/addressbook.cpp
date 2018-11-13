#include "addressbook.h"
#include "ui_addressbook.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "mainwindow.h"


AddressBookModel::AddressBookModel(QTableView *parent)
     : QAbstractTableModel(parent) {
    headers << "Label" << "Address";

    this->parent = parent;
    loadData();
}

AddressBookModel::~AddressBookModel() {
    saveData();
}

void AddressBookModel::saveData() {
    AddressBook::writeToStorage(labels);

    // Save column positions
    QSettings().setValue("addresstablegeometry", parent->horizontalHeader()->saveState());
}


void AddressBookModel::loadData() {        
    labels = AddressBook::readFromStorage();

    parent->horizontalHeader()->restoreState(QSettings().value("addresstablegeometry").toByteArray());
}

void AddressBookModel::addNewLabel(QString label, QString addr) {
    labels.push_back(QPair<QString, QString>(label, addr));
    AddressBook::writeToStorage(labels);

    dataChanged(index(0, 0), index(labels.size()-1, columnCount(index(0,0))-1));
    layoutChanged();
}

void AddressBookModel::removeItemAt(int row) {
    if (row >= labels.size())
        return;

    labels.removeAt(row);
    AddressBook::writeToStorage(labels);


    dataChanged(index(0, 0), index(labels.size()-1, columnCount(index(0,0))-1));
    layoutChanged();
}

QPair<QString, QString> AddressBookModel::itemAt(int row) {
    if (row >= labels.size()) return QPair<QString, QString>();

    return labels.at(row);
}


int AddressBookModel::rowCount(const QModelIndex&) const {
    return labels.size();
}

int AddressBookModel::columnCount(const QModelIndex&) const {
    return headers.size();
}


QVariant AddressBookModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        switch(index.column()) {
            case 0: return labels.at(index.row()).first;
            case 1: return labels.at(index.row()).second;
        }
    }
    return QVariant();
}  


QVariant AddressBookModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return headers.at(section);
    }

    return QVariant();
}

void AddressBook::open(MainWindow* parent, QLineEdit* target) {
    QDialog d(parent);
    Ui_addressBook ab;
    ab.setupUi(&d);
    Settings::saveRestore(&d);

    AddressBookModel model(ab.addresses);
    ab.addresses->setModel(&model);

    // If there is no target, the we'll call the button "Ok", else "Pick"
    if (target != nullptr) {
        ab.buttonBox->button(QDialogButtonBox::Ok)->setText("Pick");
    } 

    // Connect the dialog's closing to updating the label address completor
    QObject::connect(&d, &QDialog::finished, [=] (auto) { parent->updateLabelsAutoComplete(); });

    // If there is a target then make it the addr for the "Add to" button
    if (target != nullptr && Settings::isValidAddress(target->text())) {
        ab.addr->setText(target->text());
        ab.label->setFocus();
    }

    // Add new address button
    QObject::connect(ab.addNew, &QPushButton::clicked, [&] () {
        auto addr = ab.addr->text().trimmed();
        if (!addr.isEmpty() && !ab.label->text().isEmpty()) {
            // Test if address is valid.
            if (!Settings::isValidAddress(addr)) {
                QMessageBox::critical(parent, "Address Format Error", addr + " doesn't seem to be a valid Zcash address.", QMessageBox::Ok);
            } else {
                model.addNewLabel(ab.label->text(), ab.addr->text());
            }
        }
    });

    auto fnSetTargetLabelAddr = [=] (QLineEdit* target, QString label, QString addr) {
        target->setText(label % "/" % addr);
    };

    // Double-Click picks the item
    QObject::connect(ab.addresses, &QTableView::doubleClicked, [&] (auto index) {
        if (index.row() < 0) return;

        QString lbl  = model.itemAt(index.row()).first;
        QString addr = model.itemAt(index.row()).second;
        d.accept();
        fnSetTargetLabelAddr(target, lbl, addr);
    });

    // Right-Click
    ab.addresses->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ab.addresses, &QTableView::customContextMenuRequested, [&] (QPoint pos) {
        QModelIndex index = ab.addresses->indexAt(pos);

        if (index.row() < 0) return;

        QString lbl  = model.itemAt(index.row()).first;
        QString addr = model.itemAt(index.row()).second;

        QMenu menu(parent);

        if (target != nullptr) {
            menu.addAction("Pick", [&] () {
                d.accept();
                fnSetTargetLabelAddr(target, lbl, addr);
            });
        }

        menu.addAction("Copy Address", [&] () {
            QGuiApplication::clipboard()->setText(addr);            
            parent->ui->statusBar->showMessage("Copied to clipboard", 3 * 1000);
        });

        menu.addAction("Delete Label", [&] () {
            model.removeItemAt(index.row());
        });

        menu.exec(ab.addresses->viewport()->mapToGlobal(pos));    
    });

    if (d.exec() == QDialog::Accepted && target != nullptr) {
        auto selection = ab.addresses->selectionModel();
        if (selection->hasSelection()) {
            auto item = model.itemAt(selection->selectedRows().at(0).row());
            fnSetTargetLabelAddr(target, item.first, item.second);
        }
    };
}

QList<QPair<QString, QString>> AddressBook::readFromStorage() {
    QFile file(AddressBook::writeableFile());
    
    QList<QPair<QString, QString>> labels;    

    if (!file.exists()) {
        return labels;
    }

    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);    // read the data serialized from the file
    QString version;
    in >> version >> labels; 

    file.close();

    return labels;
}


void AddressBook::writeToStorage(QList<QPair<QString, QString>> labels) {
    QFile file(AddressBook::writeableFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate);
    QDataStream out(&file);   // we will serialize the data into the file
    out << QString("v1") << labels;
    file.close();
}

QString AddressBook::writeableFile() {
    auto filename = QStringLiteral("addresslabels.dat");

    auto dir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!dir.exists())
        QDir().mkpath(dir.absolutePath());

    if (Settings::getInstance()->isTestnet()) {
        return dir.filePath("testnet-" % filename);
    } else {
        return dir.filePath(filename);
    }
}
