#include "connection.h"
#include "mainwindow.h"
#include "settings.h"
#include "ui_connection.h"
#include "rpc.h"

#include "precompiled.h"

using json = nlohmann::json;

class LoadingDialog : public QDialog {
    //Q_OBJECT
public:
    LoadingDialog(QWidget* parent);
    ~LoadingDialog();
public slots:
    void reject();
};

LoadingDialog::LoadingDialog(QWidget* parent) : QDialog(parent) {}
LoadingDialog::~LoadingDialog() {}
void LoadingDialog::reject() {
    //event->ignore();
}

ConnectionLoader::ConnectionLoader(MainWindow* main) {
    this->main = main;

    d = new LoadingDialog(main);
    connD = new Ui_ConnectionDialog();
    connD->setupUi(d);

    // Center on screen
    QRect screenGeometry = QApplication::desktop()->screenGeometry(d);
    int x = (screenGeometry.width() - d->width()) / 2;
    int y = (screenGeometry.height() - d->height()) / 2;
    d->move(x, y);
    connD->buttonBox->setEnabled(false);
    d->show();
}

ConnectionLoader::~ConnectionLoader() {
    delete d;
    delete connD;
}

void ConnectionLoader::getConnection(std::function<void(RPC*)> cb) {    

    // Priority 1: Try to connect to detect zcash.conf and connect to it.
    bool isZcashConfPresent = false;
    auto conn = autoDetectZcashConf();

    // If not autodetected, go and read the UI Settings
    if (conn.get() != nullptr)  {
        isZcashConfPresent = true;
    } else {
        conn = loadFromSettings();

        if (conn.get() == nullptr) {
            // Nothing configured, show an error
            auto explanation = QString()
                    % "A zcash.conf was not found on this machine.\n\n"
                    % "If you are connecting to a remote/non-standard node " 
                    % "please set the host/port and user/password in the File->Settings menu.";

            QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
            connD->icon->setPixmap(icon.pixmap(64, 64));
            connD->status->setText(explanation);
            connD->progressBar->setValue(0);

            connD->buttonBox->setEnabled(true);
            cb(nullptr);
        }
    }

    QNetworkAccessManager* client = new QNetworkAccessManager(main);
         
    QUrl myurl;
    myurl.setScheme("http");
    myurl.setHost(Settings::getInstance()->getHost());
    myurl.setPort(Settings::getInstance()->getPort().toInt());

    QNetworkRequest* request = new QNetworkRequest();
    request->setUrl(myurl);
    request->setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    
    QString headerData = "Basic " + Settings::getInstance()->getUsernamePassword().toLocal8Bit().toBase64();
    request->setRawHeader("Authorization", headerData.toLocal8Bit());

    auto connection = new Connection(client, request);
    json payload = {
        {"jsonrpc", "1.0"},
        {"id", "someid"},
        {"method", "getinfo"}
    };
    connection->doRPC(payload,
        [=] (auto result) {
            // Success
            d->close();
            cb(new RPC(connection, main));
        },
        [=] (auto err, auto res) {
            // Failed
            auto explanation = QString()
                    % (isZcashConfPresent ? "A zcash.conf file was found, but a" : "A")
                    % " connection to zcashd could not be established.\n\n"
                    % "If you are connecting to a remote/non-standard node " 
                    % "please set the host/port and user/password in the File->Settings menu.";

            QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
            connD->icon->setPixmap(icon.pixmap(64, 64));
            connD->status->setText(explanation);
            connD->progressBar->setValue(0);

            connD->buttonBox->setEnabled(true);
        }
    );
}


/**
 * Try to automatically detect a zcash.conf file in the correct location and load parameters
 */ 
std::shared_ptr<ConnectionConfig*> ConnectionLoader::autoDetectZcashConf() {    

#ifdef Q_OS_LINUX
    auto confLocation = QStandardPaths::locate(QStandardPaths::HomeLocation, ".zcash/zcash.conf");
#elif defined(Q_OS_DARWIN)
    auto confLocation = QStandardPaths::locate(QStandardPaths::HomeLocation, "/Library/Application Support/Zcash/zcash.conf");
#else
    auto confLocation = QStandardPaths::locate(QStandardPaths::AppDataLocation, "../../Zcash/zcash.conf");
#endif

    confLocation = QDir::cleanPath(confLocation);

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

    return std::make_shared<ConnectionConfig*>(zcashconf);
}

/**
 * Load connection settings from the UI, which indicates an unknown, external zcashd
 */ 
std::shared_ptr<ConnectionConfig*> ConnectionLoader::loadFromSettings() {
    // Load from the QT Settings. 
    QSettings s;
    
    auto host        = s.value("connection/host").toString();
    auto port        = s.value("connection/port").toString();
    auto username    = s.value("connection/rpcuser").toString();
    auto password    = s.value("connection/rpcpassword").toString();    

    if (username.isEmpty() || password.isEmpty())
        return nullptr;

    auto uiConfig = new ConnectionConfig{ host, port, username, password, ConnectionType::UISettingsZCashD };

    return std::make_shared<ConnectionConfig*>(uiConfig);
}








Connection::Connection(QNetworkAccessManager* c, QNetworkRequest* r) {
    this->restclient  = c;
    this->request = r;
}

Connection::~Connection() {
    delete restclient;
    delete request;
}



void Connection::doRPC(const json& payload, const std::function<void(json)>& cb, 
                       const std::function<void(QNetworkReply::NetworkError, const json&)>& ne) {
    QNetworkReply *reply = restclient->post(*request, QByteArray::fromStdString(payload.dump()));

    QObject::connect(reply, &QNetworkReply::finished, [=] {
        reply->deleteLater();
        
        if (reply->error() != QNetworkReply::NoError) {
            auto parsed = json::parse(reply->readAll(), nullptr, false);
            ne(reply->error(), parsed);
            
            return;
        } 
        
        auto parsed = json::parse(reply->readAll(), nullptr, false);
        if (parsed.is_discarded()) {
            ne(reply->error(), "Unknown error");
        }
        
        cb(parsed["result"]);        
    });
}
