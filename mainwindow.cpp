#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_about.h"
#include "ui_settings.h"
#include "rpc.h"
#include "balancestablemodel.h"
#include "settings.h"

#include "precompiled.h"
#include "ui_about.h"

using json = nlohmann::json;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Load settings
    settings = new Settings();

    // Status Bar
    statusLabel = new QLabel();
    ui->statusBar->addPermanentWidget(statusLabel);
    statusIcon = new QLabel();
    ui->statusBar->addPermanentWidget(statusIcon);
    
	// Set up File -> Settings action
	QObject::connect(ui->actionSettings, &QAction::triggered, [=]() {
		QDialog settingsDialog(this);
		Ui_Settings settings;
		settings.setupUi(&settingsDialog);

		QIntValidator validator(0, 65535);
		settings.port->setValidator(&validator);

		// Load previous values into the dialog		
		settings.hostname	->setText(this->getSettings()->getHost());
		settings.port		->setText(this->getSettings()->getPort());
		settings.rpcuser	->setText(this->getSettings()->getUsernamePassword().split(":")[0]);
		settings.rpcpassword->setText(this->getSettings()->getUsernamePassword().split(":")[1]);
		
		if (settingsDialog.exec() == QDialog::Accepted) {
			// Save settings
			QSettings s;
			s.setValue("connection/host", settings.hostname->text());
			s.setValue("connection/port", settings.port->text());
			s.setValue("connection/rpcuser", settings.rpcuser->text());
			s.setValue("connection/rpcpassword", settings.rpcpassword->text());

			s.sync();

			// Then refresh everything
			this->getSettings()->refresh();
			this->rpc->reloadConnectionInfo();
			this->rpc->refresh();
		};
	});

    // Set up exit action
    QObject::connect(ui->actionExit, &QAction::triggered, [=] { this->close(); });

    // Set up about action
    QObject::connect(ui->actionAbout, &QAction::triggered, [=] () {
        QDialog aboutDialog(this);
        Ui_about about;
        about.setupUi(&aboutDialog);
        aboutDialog.exec();
    });


    // Initialize to the balances tab
    ui->tabWidget->setCurrentIndex(0);

    setupSendTab();
    setupTransactionsTab();
    setupRecieveTab();
    setupBalancesTab();

    rpc = new RPC(new QNetworkAccessManager(this), this);
    rpc->refresh();
}

void MainWindow::setupBalancesTab() {
    ui->unconfirmedWarning->setVisible(false);

    // Setup context menu on balances tab
    ui->balancesTable->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->balancesTable, &QTableView::customContextMenuRequested, [=] (QPoint pos) {
        QModelIndex index = ui->balancesTable->indexAt(pos);
        if (index.row() < 0) return;

        index = index.sibling(index.row(), 0);

        QMenu menu(this);

        menu.addAction("Copy Address", [=] () {
            QClipboard *clipboard = QGuiApplication::clipboard();
            clipboard->setText(ui->balancesTable->model()->data(index).toString());            
        });

        menu.exec(ui->balancesTable->viewport()->mapToGlobal(pos));            
    });
}

void MainWindow::setupTransactionsTab() {
    // Set up context menu on transactions tab
    ui->transactionsTable->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->transactionsTable, &QTableView::customContextMenuRequested, [=] (QPoint pos) {
        QModelIndex index = ui->transactionsTable->indexAt(pos);
        if (index.row() < 0) return;

        QMenu menu(this);

        menu.addAction("View txid", [=] () {
            QMessageBox msg(this);
			msg.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
            msg.setIcon(QMessageBox::Icon::Information); 
            msg.setWindowTitle("Transaction ID");
            auto txModel = dynamic_cast<TxTableModel *>(ui->transactionsTable->model());
            msg.setText("Transaction ID: \n\n" + txModel->getTxId(index.row()));
            msg.exec();        
        });
        menu.exec(ui->transactionsTable->viewport()->mapToGlobal(pos));        
    });
}

void MainWindow::setupRecieveTab() {
    auto addNewTAddr = [=] () {
        rpc->newTaddr([=] (json reply) {
                QString addr = QString::fromStdString(reply.get<json::string_t>());

                // Just double make sure the T-address is still checked
                if (ui->rdioTAddr->isChecked()) {
                    ui->listRecieveAddresses->insertItem(0, addr);
                    ui->listRecieveAddresses->setCurrentIndex(0);

                    ui->statusBar->showMessage("Created new recieving tAddr", 10 * 1000);
                }
            });
    };

    // Connect t-addr radio button
    QObject::connect(ui->rdioTAddr, &QRadioButton::toggled, [=] (bool checked) { 
        // Whenever the T-address is selected, we generate a new address, because we don't
        // want to reuse T-addrs
        if (checked && this->rpc->getUTXOs() != nullptr) { 
            auto utxos = this->rpc->getUTXOs();
            ui->listRecieveAddresses->clear();

            std::for_each(utxos->begin(), utxos->end(), [=] (auto& utxo) {
                auto addr = utxo.address;
                if (addr.startsWith("t") && ui->listRecieveAddresses->findText(addr) < 0) {
                    ui->listRecieveAddresses->addItem(addr);
                }
            });

            addNewTAddr();
        } 
    });

    auto addNewZaddr = [=] () {
        rpc->newZaddr([=] (json reply) {
            QString addr = QString::fromStdString(reply.get<json::string_t>());
            // Make sure the RPC class reloads the Z-addrs for future use
            rpc->refreshAddresses();

            // Just double make sure the Z-address is still checked
            if (ui->rdioZAddr->isChecked()) {
                ui->listRecieveAddresses->insertItem(0, addr);
                ui->listRecieveAddresses->setCurrentIndex(0);

                ui->statusBar->showMessage("Created new zAddr", 10 * 1000);
            }
        });
    };

    auto addZAddrsToComboList = [=] (bool checked) { 
        if (checked && this->rpc->getAllZAddresses() != nullptr) { 
            auto addrs = this->rpc->getAllZAddresses();
            ui->listRecieveAddresses->clear();

            std::for_each(addrs->begin(), addrs->end(), [=] (auto addr) {
                    ui->listRecieveAddresses->addItem(addr);
            }); 

            // If z-addrs are empty, then create a new one.
            if (addrs->isEmpty()) {
                addNewZaddr();
            }
        } 
    };

    QObject::connect(ui->rdioZAddr, &QRadioButton::toggled, addZAddrsToComboList);

    // Explicitly get new address button.
    QObject::connect(ui->btnRecieveNewAddr, &QPushButton::clicked, [=] () {
        if (ui->rdioZAddr->isChecked()) {
            addNewZaddr(); 
        } else if (ui->rdioTAddr->isChecked()) {
            addNewTAddr();
        }
    });

    // Focus enter for the Recieve Tab
    QObject::connect(ui->tabWidget, &QTabWidget::currentChanged, [=] (int tab) {
        if (tab == 2) {
            // Switched to recieve tab, so update everything. 
            
            // Set the radio button to "Z-Addr", which should update the Address combo
            ui->rdioZAddr->setChecked(true);

            // And then select the first one
            ui->listRecieveAddresses->setCurrentIndex(0);
        }
    });

    QObject::connect(ui->listRecieveAddresses, 
        QOverload<const QString &>::of(&QComboBox::currentIndexChanged), [=] (const QString& addr) {
        if (addr.isEmpty()) {
            // Draw empty stuff

            ui->txtRecieve->clear();
            ui->qrcodeDisplay->clear();
            return;
        }

        ui->txtRecieve->setPlainText(addr);       
        
        QSize sz = ui->qrcodeDisplay->size();

        QPixmap pm(sz);
        pm.fill(Qt::white);
        QPainter painter(&pm);
        
        // NOTE: At this point you will use the API to get the encoding and format you want, instead of my hardcoded stuff:
        qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(addr.toUtf8().constData(), qrcodegen::QrCode::Ecc::LOW);
        const int    s      = qr.getSize()>0?qr.getSize():1;
        const double w      = sz.width();
        const double h      = sz.height();
        const double aspect = w/h;
        const double size   = ((aspect>1.0)?h:w);
        const double scale  = size/(s+2);
		const double offset = (w - size) > 0 ? (w - size) / 2 : 0;
        // NOTE: For performance reasons my implementation only draws the foreground parts in supplied color.
        // It expects background to be prepared already (in white or whatever is preferred).
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(Qt::black));
        for(int y=0; y<s; y++) {
            for(int x=0; x<s; x++) {
                const int color=qr.getModule(x, y);  // 0 for white, 1 for black
                if(0!=color) {
                    const double rx1=(x+1)*scale+ offset, ry1=(y+1)*scale;
                    QRectF r(rx1, ry1, scale, scale);
                    painter.drawRects(&r,1);
                }
            }
        }
        
        ui->qrcodeDisplay->setPixmap(pm);
    });    

}

Settings* MainWindow::getSettings() {
    return settings;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete rpc;
    delete settings;
}
