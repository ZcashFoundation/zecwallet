#include "connection.h"
#include "mainwindow.h"
#include "settings.h"
#include "ui_connection.h"
#include "rpc.h"

#include "precompiled.h"

using json = nlohmann::json;

ConnectionLoader::ConnectionLoader(MainWindow* main, RPC* rpc) {
    this->main = main;
    this->rpc  = rpc;

    d = new QDialog(main);
    connD = new Ui_ConnectionDialog();
    connD->setupUi(d);

    // Center on screen
    QRect screenGeometry = QApplication::desktop()->screenGeometry(d);
    int x = (screenGeometry.width() - d->width()) / 2;
    int y = (screenGeometry.height() - d->height()) / 2;
    d->move(x, y);
    connD->buttonBox->setEnabled(false);    
}

ConnectionLoader::~ConnectionLoader() {
    delete d;
    delete connD;
}

void ConnectionLoader::loadConnection() {    
    // Load from settings if it is a manual connection.
    if (Settings::getInstance()->getIsManualConnection()) {
        doManualConnect();
    } else {
        doAutoConnect();
    }
}

void ConnectionLoader::doAutoConnect() {
    // Priority 1: Try to connect to detect zcash.conf and connect to it.
    auto config = autoDetectZcashConf();

    if (config.get() != nullptr) {
        auto connection = makeConnection(config);
        refreshZcashdState(connection);

        return;
    } else {
        // zcash.conf was not found, so create one
        createZcashConf();
    }
}

/**
 * This will create a new zcash.conf, download zcash parameters.
 */ 
void ConnectionLoader::createZcashConf() {
    // Create zcash.conf
    {
        auto confLocation = zcashConfWritableLocation();
        qDebug() << "Creating file " << confLocation;

        QFileInfo fi(confLocation);
        QDir().mkdir(fi.dir().absolutePath());

        QFile file(confLocation);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
            qDebug() << "Could not create zcash.conf, returning";
            return;
        }
            
        QTextStream out(&file); 
        
        out << "server=1\n";
        out << "rpcuser=zec-qt-wallet\n";
        out << "rpcpassword=" % QString::number(std::rand()) << "\n";
        file.close();
    }

    // Fetch params. 
    {
        QFileInfo fi(Settings::getInstance()->getExecName());
        auto fetchParamsProgram = fi.dir().filePath("zcash-fetch-params");

        QProcess* p = new QProcess(main);
        QObject::connect(p, &QProcess::readyReadStandardOutput, [=] () {
            qDebug() << "stdout:" << p->readAllStandardOutput();
        });
        QObject::connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                         [=](int exitCode, QProcess::ExitStatus exitStatus) {
            qDebug() << "finished with code " << exitCode;
            p->deleteLater();
        });
        p->start(fetchParamsProgram);

        // stdout: "Zcash - fetch-params.sh\n\nThis script will fetch the Zcash zkSNARK parameters and verify their\nintegrity with sha256sum.\n\nIf they already exist locally, it will exit now and do nothing else.\nThe parameters are currently just under 911MB in size, so plan accordingly\nfor your bandwidth constraints. If the files are already present and\nhave the correct sha256sum, no networking is used.\n\nCreating params directory. For details about this directory, see:\n/home/adityapk/.zcash-params/README\n\n\nRetrieving (wget): https://z.cash/downloads/sprout-proving.key\n"
        // stdout: "Download successful!\n"
        // stdout: "/home/adityapk/.zcash-params/sprout-proving.key.dl: OK\n"
        // stdout: "renamed '/home/adityapk/.zcash-params/sprout-proving.key.dl' -> '/home/adityapk/.zcash-params/sprout-proving.key'\n"
        // stdout: "\nRetrieving (wget): https://z.cash/downloads/sprout-verifying.key\n"
        // stdout: "Download successful!\n"
        // stdout: "/home/adityapk/.zcash-params/sprout-verifying.key.dl: OK\n"
        // stdout: "renamed '/home/adityapk/.zcash-params/sprout-verifying.key.dl' -> '/home/adityapk/.zcash-params/sprout-verifying.key'\n"
        // stdout: "\nRetrieving (wget): https://z.cash/downloads/sapling-spend.params\n"
        // stdout: "Download successful!\n"
        // stdout: "/home/adityapk/.zcash-params/sapling-spend.params.dl: OK\n"
        // stdout: "renamed '/home/adityapk/.zcash-params/sapling-spend.params.dl' -> '/home/adityapk/.zcash-params/sapling-spend.params'\n"
        // stdout: "\nRetrieving (wget): https://z.cash/downloads/sapling-output.params\n"
        // stdout: "Download successful!\n"
        // stdout: "/home/adityapk/.zcash-params/sapling-output.params.dl: OK\n"
        // stdout: "renamed '/home/adityapk/.zcash-params/sapling-output.params.dl' -> '/home/adityapk/.zcash-params/sapling-output.params'\n"
        // stdout: "\nRetrieving (wget): https://z.cash/downloads/sprout-groth16.params\n"
        // stdout: "Download successful!\n"
        // stdout: "/home/adityapk/.zcash-params/sprout-groth16.params.dl: OK\n"
        // stdout: "renamed '/home/adityapk/.zcash-params/sprout-groth16.params.dl' -> '/home/adityapk/.zcash-params/sprout-groth16.params'\n"
        // finished with code  0
    }

    {
        startEmbeddedZcashd();

        auto config = autoDetectZcashConf();
        if (config.get() != nullptr) {
            auto connection = makeConnection(config);
            refreshZcashdState(connection);

            return;
        } else {
            qDebug() << "Coulnd't get embedded startup zcashd";
        }
    }
}

bool ConnectionLoader::startEmbeddedZcashd() {
    static bool erroredOut = false;

    if (erroredOut) {
        qDebug() << "Last zcashd start attempted errored, so not restarting";
        return false;
    }

    // Finally, start zcashd    
    qDebug() << "Starting zcashd";
    QFileInfo fi(Settings::getInstance()->getExecName());
    auto zcashdProgram = fi.dir().filePath("zcashd");

    QProcess* p = new QProcess(main);
    QObject::connect(p, &QProcess::started, [=] () {
        Settings::getInstance()->setEmbeddedZcashdRunning(true);
    });

    QObject::connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                        [=](int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << "zcashd finished with code " << exitCode;
        p->deleteLater();
    });

    QObject::connect(p, &QProcess::errorOccurred, [&] (auto error) mutable {
        qDebug() << "Couldn't start zcashd: " << error;
        erroredOut = true;
    });

    p->start(zcashdProgram);    
    return true;
}

void ConnectionLoader::doManualConnect() {
    auto config = loadFromSettings();

    if (config.get() == nullptr) {
        d->show();
        // Nothing configured, show an error
        QString explanation = QString()
                % "A manual connection was requested, but the settings are not configured.\n\n" 
                % "Please set the host/port and user/password in the File->Settings menu.";

        showError(explanation);
        doRPCSetConnection(nullptr);

        return;
    }

    auto connection = makeConnection(config);
    refreshZcashdState(connection);
}

void ConnectionLoader::doRPCSetConnection(Connection* conn) {
    rpc->setConnection(conn);
    delete this;
}

Connection* ConnectionLoader::makeConnection(std::shared_ptr<ConnectionConfig> config) {
    QNetworkAccessManager* client = new QNetworkAccessManager(main);
         
    QUrl myurl;
    myurl.setScheme("http");
    myurl.setHost(config.get()->host);
    myurl.setPort(config.get()->port.toInt());

    QNetworkRequest* request = new QNetworkRequest();
    request->setUrl(myurl);
    request->setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    
    QString userpass = config.get()->rpcuser % ":" % config.get()->rpcpassword;
    QString headerData = "Basic " + userpass.toLocal8Bit().toBase64();
    request->setRawHeader("Authorization", headerData.toLocal8Bit());    

    return new Connection(main, client, request, config);
}

void ConnectionLoader::refreshZcashdState(Connection* connection) {
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "getinfo"}
    };
    connection->doRPC(payload,
        [=] (auto) {
            // Success, hide the dialog if it was shown. 
            d->hide();
            this->doRPCSetConnection(connection);
        },
        [=] (auto reply, auto res) {
            d->show();

            auto err = reply->error();
            // Failed, see what it is. 
            //qDebug() << err << ":" << QString::fromStdString(res.dump());

            if (err == QNetworkReply::NetworkError::ConnectionRefusedError) {   
                // Start embedded zcasd
                this->showInformation("Starting Embedded zcashd");
                if (this->startEmbeddedZcashd()) {                
                    // Refresh after one second
                    QTimer::singleShot(1000, [=]() { this->refreshZcashdState(connection); });
                } else {
                    // Errored out, show error and exit
                    QString explanation = "Couldn't start zcashd";
                    this->showError(explanation);
                }

                // auto isZcashConfFound = connection->config.get()->usingZcashConf;
                // QString explanation = QString()
                //         % (isZcashConfFound ? "A zcash.conf file was found, but a" : "A") 
                //         % " connection to zcashd could not be established.\n\n"
                //         % "If you are connecting to a remote/non-standard node " 
                //         % "please set the host/port and user/password in the File->Settings menu";

                // this->showError(explanation);
            } else if (err == QNetworkReply::NetworkError::AuthenticationRequiredError) {
                QString explanation = QString() 
                        % "Authentication failed. The username / password you specified was "
                        % "not accepted by zcashd. Try changing it in the File->Settings menu";

                this->showError(explanation);
            } else if (err == QNetworkReply::NetworkError::InternalServerError && !res.is_discarded()) {
                // The server is loading, so just poll until it succeeds
                QString status = QString::fromStdString(res["error"]["message"]);
                showInformation("Your zcashd is starting up. Please wait.\n\n" % status);

                // Refresh after one second
                QTimer::singleShot(1000, [=]() { this->refreshZcashdState(connection); });
            }
        }
    );
}

void ConnectionLoader::showInformation(QString info) {
    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
    connD->icon->setPixmap(icon.pixmap(128, 128));
    connD->status->setText(info);
}

void ConnectionLoader::showError(QString explanation) {
    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
    connD->icon->setPixmap(icon.pixmap(128, 128));
    connD->status->setText(explanation);

    connD->buttonBox->setEnabled(true);
}

QString ConnectionLoader::locateZcashConfFile() {
#ifdef Q_OS_LINUX
    auto confLocation = QStandardPaths::locate(QStandardPaths::HomeLocation, ".zcash/zcash.conf");
#elif defined(Q_OS_DARWIN)
    auto confLocation = QStandardPaths::locate(QStandardPaths::HomeLocation, "/Library/Application Support/Zcash/zcash.conf");
#else
    auto confLocation = QStandardPaths::locate(QStandardPaths::AppDataLocation, "../../Zcash/zcash.conf");
#endif
    return QDir::cleanPath(confLocation);
}

QString ConnectionLoader::zcashConfWritableLocation() {
#ifdef Q_OS_LINUX
    auto confLocation = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).filePath(".zcash/zcash.conf");
#elif defined(Q_OS_DARWIN)
    auto confLocation = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).filePath("/Library/Application Support/Zcash/zcash.conf");
#else
    auto confLocation = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("../../Zcash/zcash.conf");
#endif

    return confLocation;
}

/**
 * Try to automatically detect a zcash.conf file in the correct location and load parameters
 */ 
std::shared_ptr<ConnectionConfig> ConnectionLoader::autoDetectZcashConf() {    
    auto confLocation = locateZcashConfFile();

    if (confLocation.isNull()) {
        // No zcash file, just return with nothing
        return nullptr;
    }

    QFile file(confLocation);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString();
        return nullptr;
    }

    QTextStream in(&file);

    auto zcashconf = new ConnectionConfig();
    zcashconf->host     = "127.0.0.1";
    zcashconf->connType = ConnectionType::DetectedConfExternalZcashD;
    zcashconf->usingZcashConf = true;

    Settings::getInstance()->setUsingZcashConf(confLocation);

    while (!in.atEnd()) {
        QString line = in.readLine();
        auto s = line.indexOf("=");
        QString name  = line.left(s).trimmed().toLower();
        QString value = line.right(line.length() - s - 1).trimmed();

        if (name == "rpcuser") {
            zcashconf->rpcuser = value;
        }
        if (name == "rpcpassword") {
            zcashconf->rpcpassword = value;
        }
        if (name == "rpcport") {
            zcashconf->port = value;
        }
        if (name == "testnet" &&
            value == "1"  &&
            zcashconf->port.isEmpty()) {
                zcashconf->port = "18232";
        }
    }

    // If rpcport is not in the file, and it was not set by the testnet=1 flag, then go to default
    if (zcashconf->port.isEmpty()) zcashconf->port = "8232";

    file.close();

    return std::shared_ptr<ConnectionConfig>(zcashconf);
}

/**
 * Load connection settings from the UI, which indicates an unknown, external zcashd
 */ 
std::shared_ptr<ConnectionConfig> ConnectionLoader::loadFromSettings() {
    // Load from the QT Settings. 
    QSettings s;
    
    auto host        = s.value("connection/host").toString();
    auto port        = s.value("connection/port").toString();
    auto username    = s.value("connection/rpcuser").toString();
    auto password    = s.value("connection/rpcpassword").toString();    

    if (username.isEmpty() || password.isEmpty())
        return nullptr;

    auto uiConfig = new ConnectionConfig{ host, port, username, password, false, ConnectionType::UISettingsZCashD };

    return std::shared_ptr<ConnectionConfig>(uiConfig);
}





/***********************************************************************************
 *  Connection Class
 ************************************************************************************/ 
Connection::Connection(MainWindow* m, QNetworkAccessManager* c, QNetworkRequest* r, 
                        std::shared_ptr<ConnectionConfig> conf) {
    this->restclient  = c;
    this->request     = r;
    this->config      = conf;
    this->main        = m;
}

Connection::~Connection() {
    delete restclient;
    delete request;
}

void Connection::doRPC(const json& payload, const std::function<void(json)>& cb, 
                       const std::function<void(QNetworkReply*, const json&)>& ne) {
    QNetworkReply *reply = restclient->post(*request, QByteArray::fromStdString(payload.dump()));

    QObject::connect(reply, &QNetworkReply::finished, [=] {
        reply->deleteLater();
        
        if (reply->error() != QNetworkReply::NoError) {
            auto parsed = json::parse(reply->readAll(), nullptr, false);
            ne(reply, parsed);
            
            return;
        } 
        
        auto parsed = json::parse(reply->readAll(), nullptr, false);
        if (parsed.is_discarded()) {
            ne(reply, "Unknown error");
        }
        
        cb(parsed["result"]);        
    });
}

void Connection::doRPCWithDefaultErrorHandling(const json& payload, const std::function<void(json)>& cb) {
    doRPC(payload, cb, [=] (auto reply, auto parsed) {
        if (!parsed.is_discarded() && !parsed["error"]["message"].is_null()) {
            this->showTxError(QString::fromStdString(parsed["error"]["message"]));    
        } else {
            this->showTxError(reply->errorString());
        }
    });
}

void Connection::doRPCIgnoreError(const json& payload, const std::function<void(json)>& cb) {
    doRPC(payload, cb, [=] (auto, auto) {
        // Ignored error handling
    });
}

void Connection::showTxError(const QString& error) {
    if (error.isNull()) return;

    QMessageBox::critical(main, "Transaction Error", "There was an error sending the transaction. The error was: \n\n"
        + error, QMessageBox::StandardButton::Ok);
}
