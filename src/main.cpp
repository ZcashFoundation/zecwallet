#include "mainwindow.h"
#include "settings.h"
#include "turnstile.h"

#include "version.h"

#include "precompiled.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);
    QIcon icon(":/icons/res/icon.ico");
    QApplication::setWindowIcon(icon);

    #ifdef Q_OS_LINUX
        QFontDatabase::addApplicationFont(":/fonts/res/Ubuntu-R.ttf");
        qApp->setFont(QFont("Ubuntu", 11, QFont::Normal, false));
    #endif

    // QRandomGenerator generates a secure random number, which we use to seed.
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    unsigned int seed = QRandomGenerator::securelySeeded().generate();
#else
    // This will be used only during debugging for compatibility reasons 
    unsigned int seed = std::time(0);
#endif
    std::srand(seed);


    Settings::init();

    if (argc >= 2 && QString::fromStdString(argv[1]) == "--no-embedded") {
        Settings::getInstance()->setUseEmbedded(false);
    } else {
        Settings::getInstance()->setUseEmbedded(true);
    }

    QCoreApplication::setOrganizationName("zec-qt-wallet-org");
    QCoreApplication::setApplicationName("zec-qt-wallet");

    MainWindow w;
    w.setWindowTitle("zec-qt-wallet v" + QString(APP_VERSION));
    w.show();
    
    return QApplication::exec();
}
