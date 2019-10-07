#include "addressbook.h"
#include "ui_addressbook.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "mainwindow.h"
#include "controller.h"


AddressBookModel::AddressBookModel(QTableView *parent)
     : QAbstractTableModel(parent) {
    headers << tr("Label") << tr("Address");

    this->parent = parent;
    loadData();
}

AddressBookModel::~AddressBookModel() {
    saveData();
}

void AddressBookModel::saveData() {
    // Save column positions
    QSettings().setValue("addresstablegeometry", parent->horizontalHeader()->saveState());
}


void AddressBookModel::loadData() {        
    labels = AddressBook::getInstance()->getAllAddressLabels();

    parent->horizontalHeader()->restoreState(QSettings().value("addresstablegeometry").toByteArray());
}

void AddressBookModel::addNewLabel(QString label, QString addr) {
    //labels.push_back(QPair<QString, QString>(label, addr));
    AddressBook::getInstance()->addAddressLabel(label, addr);

    labels.clear();
    labels = AddressBook::getInstance()->getAllAddressLabels();

    dataChanged(index(0, 0), index(labels.size()-1, columnCount(index(0,0))-1));
    layoutChanged();
}

void AddressBookModel::removeItemAt(int row) {
    if (row >= labels.size())
        return;

    AddressBook::getInstance()->removeAddressLabel(labels[row].first, labels[row].second);
    
    labels.clear();
    labels = AddressBook::getInstance()->getAllAddressLabels();

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


//===============
// AddressBook
//===============
void AddressBook::open(MainWindow* parent, QLineEdit* target) {
    QDialog d(parent);
    Ui_addressBook ab;
    ab.setupUi(&d);
    Settings::saveRestore(&d);

    QRegExpValidator v(QRegExp(Settings::labelRegExp), ab.label);
    ab.label->setValidator(&v);

    AddressBookModel model(ab.addresses);
    ab.addresses->setModel(&model);

    // If there is no target, the we'll call the button "Ok", else "Pick"
    if (target != nullptr) {
        ab.buttonBox->button(QDialogButtonBox::Ok)->setText(QObject::tr("Pick"));
    } 

    // Connect the dialog's closing to updating the label address completor
    QObject::connect(&d, &QDialog::finished, [=] (auto) { parent->updateLabels(); });

    // If there is a target then make it the addr for the "Add to" button
    if (target != nullptr && Settings::isValidAddress(target->text())) {
        ab.addr->setText(target->text());
        ab.label->setFocus();
    }

    // Add new address button
    QObject::connect(ab.addNew, &QPushButton::clicked, [&] () {
        auto addr = ab.addr->text().trimmed();
        QString newLabel = ab.label->text();

        if (addr.isEmpty() || newLabel.isEmpty()) {
            QMessageBox::critical(parent, QObject::tr("Address or Label Error"), 
                QObject::tr("Address or Label cannot be empty"), QMessageBox::Ok);
            return;
        }
        // Test if address is valid.
        if (!Settings::isValidAddress(addr)) {
            QMessageBox::critical(parent, QObject::tr("Address Format Error"), 
                QObject::tr("%1 doesn't seem to be a valid Zcash address.")
                    .arg(addr), 
                QMessageBox::Ok);
            return;
        } 

        // Don't allow duplicate address labels.                 
        if (!getInstance()->getAddressForLabel(newLabel).isEmpty()) {
            QMessageBox::critical(parent, QObject::tr("Label Error"), 
                QObject::tr("The label '%1' already exists. Please remove the existing label.")
                    .arg(newLabel), 
                QMessageBox::Ok);
            return;
        } 
    
        model.addNewLabel(newLabel, ab.addr->text());
    });

    // Import Button
    QObject::connect(ab.btnImport, &QPushButton::clicked, [&] () {
        // Get the import file name.
        auto fileName = QFileDialog::getOpenFileUrl(&d, QObject::tr("Import Address Book"), QUrl(), 
            "CSV file (*.csv)");
        if (fileName.isEmpty())
            return;

        QFile file(fileName.toLocalFile());
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(&d, QObject::tr("Unable to open file"), file.errorString());
            return;
        }        

        QTextStream in(&file);
        QString line;
        int numImported = 0;
        while (in.readLineInto(&line)) {
            QStringList items = line.split(",");
            if (items.size() != 2)
                continue;

            if (!Settings::isValidAddress(items.at(0)))
                continue;

            // Add label, address.
            model.addNewLabel(items.at(1), items.at(0));
            numImported++;
        }

        QMessageBox::information(&d, QObject::tr("Address Book Import Done"),
            QObject::tr("Imported %1 new Address book entries").arg(numImported));
    });

    auto fnSetTargetLabelAddr = [=] (QLineEdit* target, QString label, QString addr) {
        target->setText(label % "/" % addr);
    };

    // Double-Click picks the item
    QObject::connect(ab.addresses, &QTableView::doubleClicked, [&] (auto index) {
        // If there's no target, then double-clicking does nothing.
        if (!target)
            return;

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

        menu.addAction(QObject::tr("Copy address"), [&] () {
            QGuiApplication::clipboard()->setText(addr);            
            parent->ui->statusBar->showMessage(QObject::tr("Copied to clipboard"), 3 * 1000);
        });

        menu.addAction(QObject::tr("Delete label"), [&] () {
            model.removeItemAt(index.row());
        });

        menu.exec(ab.addresses->viewport()->mapToGlobal(pos));    
    });

    if (d.exec() == QDialog::Accepted && target != nullptr) {
        auto selection = ab.addresses->selectionModel();
        if (selection && selection->hasSelection() && selection->selectedRows().size() > 0) {
            auto item = model.itemAt(selection->selectedRows().at(0).row());
            fnSetTargetLabelAddr(target, item.first, item.second);
        }
    };

    // Refresh after the dialog is closed to update the labels everywhere.
    parent->getRPC()->refresh(true);
}

//=============
// AddressBook singleton class
//=============
AddressBook* AddressBook::getInstance() {
    if (!instance)
        instance = new AddressBook();

    return instance;
}

AddressBook::AddressBook() {
    readFromStorage();
}

void AddressBook::readFromStorage() {
    QFile file(AddressBook::writeableFile());

    if (file.exists()) {
        allLabels.clear();
        file.open(QIODevice::ReadOnly);
        QDataStream in(&file);    // read the data serialized from the file
        QString version;
        in >> version >> allLabels; 

        file.close();
    }

    // Special. 
    // Add the default ZecWallet donation address if it isn't already present
    // QList<QString> allAddresses;
    // std::transform(allLabels.begin(), allLabels.end(), 
    //     std::back_inserter(allAddresses), [=] (auto i) { return i.second; });
    // if (!allAddresses.contains(Settings::getDonationAddr(true))) {
    //     allLabels.append(QPair<QString, QString>("ZecWallet donation", Settings::getDonationAddr(true)));
    // }
}

void AddressBook::writeToStorage() {
    QFile file(AddressBook::writeableFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate);
    QDataStream out(&file);   // we will serialize the data into the file
    out << QString("v1") << allLabels;
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


// Add a new address/label to the database
void AddressBook::addAddressLabel(QString label, QString address) {
    Q_ASSERT(Settings::isValidAddress(address));

    // First, remove any existing label
    // Iterate over the list and remove the label/address
    for (int i=0; i < allLabels.size(); i++) {
        if (allLabels[i].first == label) {
            removeAddressLabel(allLabels[i].first, allLabels[i].second);
        }
    }

    allLabels.push_back(QPair<QString, QString>(label, address));
    writeToStorage();
}

// Remove a new address/label from the database
void AddressBook::removeAddressLabel(QString label, QString address) {
    // Iterate over the list and remove the label/address
    for (int i=0; i < allLabels.size(); i++) {
        if (allLabels[i].first == label && allLabels[i].second == address) {
            allLabels.removeAt(i);
            writeToStorage();
            return;
        }
    }
}

void AddressBook::updateLabel(QString oldlabel, QString address, QString newlabel) {
    // Iterate over the list and update the label/address
    for (int i = 0; i < allLabels.size(); i++) {
        if (allLabels[i].first == oldlabel && allLabels[i].second == address) {
            allLabels[i].first = newlabel;
            writeToStorage();
            return;
        }
    }
}

// Read all addresses
const QList<QPair<QString, QString>>& AddressBook::getAllAddressLabels() {
    if (allLabels.isEmpty()) {
        readFromStorage();
    }
    return allLabels;
}

// Get the label for an address
QString AddressBook::getLabelForAddress(QString addr) {
    for (auto i : allLabels) {
        if (i.second == addr)
            return i.first;
    }

    return "";
}

// Get the address for a label
QString AddressBook::getAddressForLabel(QString label) {
    for (auto i: allLabels) {
        if (i.first == label)
            return i.second;
    }

    return "";
}

QString AddressBook::addLabelToAddress(QString addr) {
    QString label = AddressBook::getInstance()->getLabelForAddress(addr);
    if (!label.isEmpty())
        return label + "/" + addr;
    else
        return addr;
}

QString AddressBook::addressFromAddressLabel(const QString& lblAddr) { 
    return lblAddr.trimmed().split("/").last(); 
}

AddressBook* AddressBook::instance = nullptr;
