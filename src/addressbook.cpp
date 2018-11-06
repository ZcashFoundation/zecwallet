#include "addressbook.h"
#include "ui_addressbook.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "mainwindow.h"
#include "utils.h"

AddressBookModel::AddressBookModel(QTableView *parent)
     : QAbstractTableModel(parent) {
    headers << "Label" << "Address";

    this->parent = parent;
    loadDataFromStorage();
}

AddressBookModel::~AddressBookModel() {
    if (labels != nullptr)
        saveDataToStorage();

    delete labels;
}

void AddressBookModel::saveDataToStorage() {
    QFile file(writeableFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate);
    QDataStream out(&file);   // we will serialize the data into the file
    out << QString("v1") << *labels;
    file.close();

    // Save column positions
    QSettings().setValue("addresstablegeometry", parent->horizontalHeader()->saveState());
}


void AddressBookModel::loadDataFromStorage() {
    QFile file(writeableFile());
    
    delete labels;
    labels = new QList<QPair<QString, QString>>();    

    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);    // read the data serialized from the file
    QString version;
    in >> version >> *labels; 

    file.close();

    parent->horizontalHeader()->restoreState(QSettings().value("addresstablegeometry").toByteArray());
}

void AddressBookModel::addNewLabel(QString label, QString addr) {
    labels->push_back(QPair<QString, QString>(label, addr));

    dataChanged(index(0, 0), index(labels->size()-1, columnCount(index(0,0))-1));
    layoutChanged();
}

void AddressBookModel::removeItemAt(int row) {
    if (row >= labels->size())
        return;
    labels->removeAt(row);

    dataChanged(index(0, 0), index(labels->size()-1, columnCount(index(0,0))-1));
    layoutChanged();
}

QPair<QString, QString> AddressBookModel::itemAt(int row) {
    if (row >= labels->size()) return QPair<QString, QString>();

    return labels->at(row);
}

QString AddressBookModel::writeableFile() {
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

int AddressBookModel::rowCount(const QModelIndex&) const {
    if (labels == nullptr) return 0;
    return labels->size();
}

int AddressBookModel::columnCount(const QModelIndex&) const {
    return headers.size();
}


QVariant AddressBookModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        switch(index.column()) {
            case 0: return labels->at(index.row()).first;
            case 1: return labels->at(index.row()).second;
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

    // If there is a target then make it the addr for the "Add to" button
    if (target != nullptr && Utils::isValidAddress(target->text())) {
        ab.addr->setText(target->text());
        ab.label->setFocus();
    }

    // Add new address button
    QObject::connect(ab.addNew, &QPushButton::clicked, [&] () {
        auto addr = ab.addr->text().trimmed();
        if (!addr.isEmpty() && !ab.label->text().isEmpty()) {
            // Test if address is valid.
            if (!Utils::isValidAddress(addr)) {
                QMessageBox::critical(parent, "Address Format Error", addr + " doesn't seem to be a valid Zcash address.", QMessageBox::Ok);
            } else {
                model.addNewLabel(ab.label->text(), ab.addr->text());
            }
        }
    });

    // Double-Click picks the item
    QObject::connect(ab.addresses, &QTableView::doubleClicked, [&] (auto index) {
        if (index.row() < 0) return;

        QString addr = model.itemAt(index.row()).second;
        d.accept();
        target->setText(addr);
    });

    // Right-Click
    ab.addresses->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ab.addresses, &QTableView::customContextMenuRequested, [&] (QPoint pos) {
        QModelIndex index = ab.addresses->indexAt(pos);

        if (index.row() < 0) return;

        QString addr = model.itemAt(index.row()).second;

        QMenu menu(parent);

        if (target != nullptr) {
            menu.addAction("Pick", [&] () {
                target->setText(addr);
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
            target->setText(model.itemAt(selection->selectedRows().at(0).row()).second);
        }
    };
}