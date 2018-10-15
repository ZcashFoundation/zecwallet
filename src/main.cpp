#include "mainwindow.h"
#include "precompiled.h"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QCoreApplication::setOrganizationName("adityapk");
	QCoreApplication::setOrganizationDomain("adityapk.com");
	QCoreApplication::setApplicationName("zcash-qt-wallet");

	QApplication a(argc, argv);
    #ifdef Q_OS_LINUX
        QFontDatabase::addApplicationFont(":/fonts/res/Ubuntu-R.ttf");
        qApp->setFont(QFont("Ubuntu", 11, QFont::Normal, false));
    #endif

    MainWindow w;
    w.show();
    
    return QApplication::exec();
}
