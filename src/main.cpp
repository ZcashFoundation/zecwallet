#include "mainwindow.h"
#include "precompiled.h"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);

    #ifdef Q_OS_LINUX
        QFontDatabase::addApplicationFont(":/fonts/res/Ubuntu-R.ttf");
        qApp->setFont(QFont("Ubuntu", 11, QFont::Normal, false));
    #endif

	QCoreApplication::setOrganizationName("adityapk");
	QCoreApplication::setOrganizationDomain("adityapk.com");
	QCoreApplication::setApplicationName("zcash-qt-wallet");


    MainWindow w;
    w.show();
    
    return QApplication::exec();
}
