#include "mainwindow.h"
#include "settings.h"
#include "turnstile.h"

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

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    Settings::init();
    Settings::getInstance()->setExecName(argv[0]);

    if (argc >= 2 && QString::fromStdString(argv[1]) == "--no-embedded") {
        Settings::getInstance()->setUseEmbedded(false);
    } else {
        Settings::getInstance()->setUseEmbedded(true);
    }

    // No embedded support on Mac yet. 
#if defined(Q_OS_DARWIN)    
    Settings::getInstance()->setUseEmbedded(false);
#endif

    QCoreApplication::setOrganizationName("zec-qt-wallet-org");
    QCoreApplication::setApplicationName("zec-qt-wallet");

    MainWindow w;
    w.setWindowTitle("zec-qt-wallet v" + QString(APP_VERSION));
    w.show();
    
    return QApplication::exec();
}
