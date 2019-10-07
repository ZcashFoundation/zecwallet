#include "turnstile.h"
#include "mainwindow.h"
#include "balancestablemodel.h"
#include "controller.h"
#include "settings.h"
#include "ui_migration.h"


using json = nlohmann::json;

// Need at least 0.0005 ZEC for this
double Turnstile::minMigrationAmount = 0.0005;

// Methods for zcashd native Migration
void Turnstile::showZcashdMigration(MainWindow* parent) {
    // If it is not enabled, don't show the dialog
    if (! parent->getRPC()->getMigrationStatus()->available)
        return;

    Ui_MigrationDialog md;
    QDialog d(parent);
    md.setupUi(&d);
    Settings::saveRestore(&d);

    MigrationTxns model(md.tblTxids, parent->getRPC()->getMigrationStatus()->txids);
    md.tblTxids->setModel(&model);

    // Table right click
    md.tblTxids->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(md.tblTxids, &QTableView::customContextMenuRequested, [=, &model] (QPoint pos) {
        QModelIndex index = md.tblTxids->indexAt(pos);
        if (index.row() < 0) return;

        QMenu menu(parent);
        QString txid = model.getTxid(index.row());

        menu.addAction("Copy txid", [=]() {
            QGuiApplication::clipboard()->setText(txid);
        });

        menu.addAction(QObject::tr("View on block explorer"), [=] () {
            QString url;
            if (Settings::getInstance()->isTestnet()) {
                url = "https://explorer.testnet.z.cash/tx/" + txid;
            } else {
                url = "https://explorer.zcha.in/transactions/" + txid;
            }
            QDesktopServices::openUrl(QUrl(url));
        });

        menu.exec(md.tblTxids->viewport()->mapToGlobal(pos));
    });

    auto* status = parent->getRPC()->getMigrationStatus();

    md.chkEnabled->setChecked(status->enabled);
    md.lblSaplingAddress->setText(status->saplingAddress);
    md.lblUnMigrated->setText(Settings::getZECDisplayFormat(status->unmigrated));
    md.lblMigrated->setText(Settings::getZECDisplayFormat(status->migrated));

    if (d.exec() == QDialog::Accepted) {
        // Update the migration status if it changed
        if (md.chkEnabled->isChecked() != status->enabled) {
            parent->getRPC()->setMigrationStatus(md.chkEnabled->isChecked());
        }
    }
}


MigrationTxns::MigrationTxns(QTableView *parent, QList<QString> txids)
     : QAbstractTableModel(parent) {
    headers << tr("Migration Txids");
    this->txids = txids;
}


int MigrationTxns::rowCount(const QModelIndex&) const {
    return txids.size();
}

int MigrationTxns::columnCount(const QModelIndex&) const {
    return headers.size();
}

QString MigrationTxns::getTxid(int row) const {
    return txids.at(row);
}

QVariant MigrationTxns::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        switch(index.column()) {
            case 0: return txids.at(index.row());
        }
    }
    return QVariant();
}  


QVariant MigrationTxns::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return headers.at(section);
    }

    return QVariant();
}
