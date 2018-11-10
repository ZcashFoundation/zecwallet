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
    QPixmap logo(":/img/res/logobig.gif");
    connD->topIcon->setBasePixmap(logo.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

ConnectionLoader::~ConnectionLoader() {    
    delete d;
    delete connD;
}

void ConnectionLoader::loadConnection() {
    QTimer::singleShot(1, [=]() { this->doAutoConnect(); });
    d->exec();
}

void ConnectionLoader::doAutoConnect() {
    // Priority 1: Try to connect to detect zcash.conf and connect to it.
    auto config = autoDetectZcashConf();

    if (config.get() != nullptr) {
        auto connection = makeConnection(config);

        refreshZcashdState(connection, [=] () {
            // Refused connection. So try and start embedded zcashd
            if (Settings::getInstance()->useEmbedded()) {
                this->showInformation("Starting Embedded zcashd");
                if (this->startEmbeddedZcashd()) {
                    // Embedded zcashd started up. Wait a second and then refresh the connection
                    QTimer::singleShot(1000, [=]() { doAutoConnect(); } );
                } else {
                    // Errored out, show error and exit
                    QString explanation = QString() % "Couldn't start the embedded zcashd.\n\n" %
                                        "Maybe the zcash-params are corrupt? Please delete your zcash-params directory and restart.\n\n" %
                                        (ezcashd ? "The process returned:\n\n" % ezcashd->errorString() : QString(""));
                    this->showError(explanation);
                }
            } else {
                // zcash.conf exists, there's no connection, and the user asked us not to start zcashd. Error!
                QString explanation = QString() % "Couldn't connect to zcashd configured in zcash.conf.\n\n" %
                                      "Not starting embedded zcashd because --no-embedded was passed";
                this->showError(explanation);
            }
        });
    } else {
        if (Settings::getInstance()->useEmbedded()) {
            // zcash.conf was not found, so create one
            createZcashConf();
        } else {
            // Fall back to manual connect
            doManualConnect();
        }
    } 
}

QString randomPassword() {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    const int passwordLength = 10;
    char* s = new char[passwordLength + 1];

    for (int i = 0; i < passwordLength; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[passwordLength] = 0;
    return QString::fromStdString(s);
}

/**
 * This will create a new zcash.conf, download zcash parameters.
 */ 
void ConnectionLoader::createZcashConf() {
    // Fetch params. After params are fetched, create the zcash.conf file and 
    // try loading the connection again
    downloadParams([=] () { 
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
        out << "rpcpassword=" % randomPassword() << "\n";
        file.close();

        this->doAutoConnect();
    });    
}


void ConnectionLoader::downloadParams(std::function<void(void)> cb) {    
    // Add all the files to the download queue
    downloadQueue = new QQueue<QUrl>();
    client = new QNetworkAccessManager(main);   
    
    downloadQueue->enqueue(QUrl("https://z.cash/downloads/sapling-output.params"));
    downloadQueue->enqueue(QUrl("https://z.cash/downloads/sapling-spend.params"));    
    downloadQueue->enqueue(QUrl("https://z.cash/downloads/sprout-proving.key"));
    downloadQueue->enqueue(QUrl("https://z.cash/downloads/sprout-verifying.key"));
    downloadQueue->enqueue(QUrl("https://z.cash/downloads/sprout-groth16.params"));

    doNextDownload(cb);    
}

void ConnectionLoader::doNextDownload(std::function<void(void)> cb) {
    auto fnSaveFileName = [&] (QUrl url) {
        QString path = url.path();
        QString basename = QFileInfo(path).fileName();

        return basename;
    };

    if (downloadQueue->isEmpty()) {
        delete downloadQueue;
        client->deleteLater();

        this->showInformation("All Downloads Finished Successfully!");
        cb();
        return;
    }

    QUrl url = downloadQueue->dequeue();
    int filesRemaining = downloadQueue->size();

    QString filename = fnSaveFileName(url);
    QString paramsDir = zcashParamsDir();

    if (QFile(QDir(paramsDir).filePath(filename)).exists()) {
        qDebug() << filename << " already exists, skipping ";
        doNextDownload(cb);

        return;
    }

    // The downloaded file is written to a new name, and then renamed when the operation completes.
    currentOutput = new QFile(QDir(paramsDir).filePath(filename + ".part"));   

    if (!currentOutput->open(QIODevice::WriteOnly)) {
        this->showError("Couldn't download params. Please check the help site for more info.");
    }
    qDebug() << "Downloading " << url << " to " << filename;
    
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    currentDownload = client->get(request);
    downloadTime.start();
    
    // Download Progress
    QObject::connect(currentDownload, &QNetworkReply::downloadProgress, [=] (auto done, auto total) {
        // calculate the download speed
        double speed = done * 1000.0 / downloadTime.elapsed();
        QString unit;
        if (speed < 1024) {
            unit = "bytes/sec";
        } else if (speed < 1024*1024) {
            speed /= 1024;
            unit = "kB/s";
        } else {
            speed /= 1024*1024;
            unit = "MB/s";
        }

        this->showInformation(
            "Downloading " % filename % (filesRemaining > 1 ? " ( +" % QString::number(filesRemaining)  % " more remaining )" : QString("")),
            QString::number(done/1024/1024, 'f', 0) % "MB of " % QString::number(total/1024/1024, 'f', 0) + "MB at " % QString::number(speed, 'f', 2) % unit);
    });
    
    // Download Finished
    QObject::connect(currentDownload, &QNetworkReply::finished, [=] () {
        // Rename file
        currentOutput->rename(QDir(paramsDir).filePath(filename));

        currentOutput->close();
        currentDownload->deleteLater();
        currentOutput->deleteLater();

        if (currentDownload->error()) {
            this->showError("Downloading " + filename + " failed/ Please check the help site for more info");                
        } else {
            doNextDownload(cb);
        }
    });

    // Download new data available. 
    QObject::connect(currentDownload, &QNetworkReply::readyRead, [=] () {
        currentOutput->write(currentDownload->readAll());
    });    
}

bool ConnectionLoader::startEmbeddedZcashd() {
    if (!Settings::getInstance()->useEmbedded()) 
        return false;

    if (ezcashd != nullptr) {
        if (ezcashd->state() == QProcess::NotRunning) {
            qDebug() << "Process started and then crashed";
            return false;
        } else {
            return true;
        }        
    }

    // Finally, start zcashd    
    QFileInfo fi(Settings::getInstance()->getExecName());
#ifdef Q_OS_LINUX
    auto zcashdProgram = fi.dir().absoluteFilePath("zcashd");
#elif defined(Q_OS_DARWIN)
    auto zcashdProgram = fi.dir().absoluteFilePath("zcashd");
#else
    auto zcashdProgram = fi.dir().absoluteFilePath("zcashd.exe");
#endif
    
    if (!QFile(zcashdProgram).exists()) {
        qDebug() << "Can't find zcashd at " << zcashdProgram;
        return false;
    }

    ezcashd = new QProcess(main);    
    QObject::connect(ezcashd, &QProcess::started, [=] () {
        //qDebug() << "zcashd started";
    });

    QObject::connect(ezcashd, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                        [=](int, QProcess::ExitStatus) {
        //qDebug() << "zcashd finished with code " << exitCode << "," << exitStatus;
    });

    QObject::connect(ezcashd, &QProcess::errorOccurred, [&] (auto) {
        //qDebug() << "Couldn't start zcashd: " << error;
    });

#ifdef Q_OS_LINUX
    ezcashd->start(zcashdProgram);
#elif defined(Q_OS_DARWIN)
    ezcashd->start(zcashdProgram);
#else
    ezcashd->setWorkingDirectory(fi.dir().absolutePath());
    ezcashd->start("zcashd.exe");
#endif // Q_OS_LINUX


    return true;
}

void ConnectionLoader::doManualConnect() {
    auto config = loadFromSettings();

    if (!config) {
        // Nothing configured, show an error
        QString explanation = QString()
                % "A manual connection was requested, but the settings are not configured.\n\n" 
                % "Please set the host/port and user/password in the Edit->Settings menu.";

        showError(explanation);
        doRPCSetConnection(nullptr);

        return;
    }

    auto connection = makeConnection(config);
    refreshZcashdState(connection, [=] () {
        QString explanation = QString()
                % "Could not connect to zcashd configured in settings.\n\n" 
                % "Please set the host/port and user/password in the Edit->Settings menu.";

        showError(explanation);
        doRPCSetConnection(nullptr);

        return;
    });
}

void ConnectionLoader::doRPCSetConnection(Connection* conn) {
    rpc->setEZcashd(ezcashd);
    rpc->setConnection(conn);
    
    d->accept();

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

void ConnectionLoader::refreshZcashdState(Connection* connection, std::function<void(void)> refused) {
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
            // Failed, see what it is. 
            auto err = reply->error();
            //qDebug() << err << ":" << QString::fromStdString(res.dump());

            if (err == QNetworkReply::NetworkError::ConnectionRefusedError) {   
                refused();
            } else if (err == QNetworkReply::NetworkError::AuthenticationRequiredError) {
                QString explanation = QString() 
                        % "Authentication failed. The username / password you specified was "
                        % "not accepted by zcashd. Try changing it in the Edit->Settings menu";

                this->showError(explanation);
            } else if (err == QNetworkReply::NetworkError::InternalServerError && 
                    !res.is_discarded()) {
                // The server is loading, so just poll until it succeeds
                QString status    = QString::fromStdString(res["error"]["message"]);
                this->showInformation("Your zcashd is starting up. Please wait.", status);

                // Refresh after one second
                QTimer::singleShot(1000, [=]() { this->refreshZcashdState(connection, refused); });
            }
        }
    );
}

void ConnectionLoader::showInformation(QString info, QString detail) {
    connD->status->setText(info);
    connD->statusDetail->setText(detail);
}

/**
 * Show error will close the loading dialog and show an error. 
*/
void ConnectionLoader::showError(QString explanation) {    
    rpc->setEZcashd(nullptr);
    rpc->noConnection();

    QMessageBox::critical(main, "Connection Error", explanation, QMessageBox::Ok);
    d->close();
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

QString ConnectionLoader::zcashParamsDir() {
    #ifdef Q_OS_LINUX
    auto paramsLocation = QDir(QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).filePath(".zcash-params"));
#elif defined(Q_OS_DARWIN)
    auto paramsLocation = QDir(QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).filePath("/Library/Application Support/ZcashParams"));
#else
    auto paramsLocation = QDir(QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("../../ZcashParams"));
#endif

    if (!paramsLocation.exists()) {
        QDir().mkpath(paramsLocation.absolutePath());
    }

    return paramsLocation.absolutePath();
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
    if (shutdownInProgress) {
        // Ignoring RPC because shutdown in progress
        return;
    }

    QNetworkReply *reply = restclient->post(*request, QByteArray::fromStdString(payload.dump()));

    QObject::connect(reply, &QNetworkReply::finished, [=] {
        reply->deleteLater();
        if (shutdownInProgress) {
            // Ignoring callback because shutdown in progress
            return;
        }
        
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

/**
 * Prevent all future calls from going through
 */ 
void Connection::shutdown() {
    shutdownInProgress = true;
}
