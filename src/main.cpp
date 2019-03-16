#include "mainwindow.h"
#include "rpc.h"
#include "settings.h"
#include "turnstile.h"

#include "version.h"

class SignalHandler 
{
public:
    SignalHandler(int mask = DEFAULT_SIGNALS);
    virtual ~SignalHandler();

    enum SIGNALS
    {
        SIG_UNHANDLED   = 0,    // Physical signal not supported by this class
        SIG_NOOP        = 1,    // The application is requested to do a no-op (only a target that platform-specific signals map to when they can't be raised anyway)
        SIG_INT         = 2,    // Control+C (should terminate but consider that it's a normal way to do so; can delay a bit)
        SIG_TERM        = 4,    // Control+Break (should terminate now without regarding the consquences)
        SIG_CLOSE       = 8,    // Container window closed (should perform normal termination, like Ctrl^C) [Windows only; on Linux it maps to SIG_TERM]
        SIG_RELOAD      = 16,   // Reload the configuration [Linux only, physical signal is SIGHUP; on Windows it maps to SIG_NOOP]
        DEFAULT_SIGNALS = SIG_INT | SIG_TERM | SIG_CLOSE,
    };
    static const int numSignals = 6;

    virtual bool handleSignal(int signal) = 0;

private:
    int _mask;
};

#include <assert.h>

#ifndef _WIN32

#include <signal.h>

#else

#endif //!_WIN32

// There can be only ONE SignalHandler per process
SignalHandler* g_handler(NULL);

#ifdef _WIN32

#else //_WIN32

void POSIX_handleFunc(int);
int POSIX_physicalToLogical(int);
int POSIX_logicalToPhysical(int);

#endif //_WIN32

SignalHandler::SignalHandler(int mask) : _mask(mask)
{
    assert(g_handler == NULL);
    g_handler = this;

#ifdef _WIN32
    
#endif //_WIN32

    for (int i=0;i<numSignals;i++)
    {
        int logical = 0x1 << i;
        if (_mask & logical)
        {
#ifdef _WIN32
            
#else
            int sig = POSIX_logicalToPhysical(logical);
            bool failed = signal(sig, POSIX_handleFunc) == SIG_ERR;
            assert(!failed);
            (void)failed; // Silence the warning in non _DEBUG; TODO: something better

#endif //_WIN32
        }
    }

}

SignalHandler::~SignalHandler()
{
#ifdef _WIN32
    
#else
    for (int i=0;i<numSignals;i++)
    {
        int logical = 0x1 << i;
        if (_mask & logical)
        {
            signal(POSIX_logicalToPhysical(logical), SIG_DFL);
        }
    }
#endif //_WIN32
}


#ifdef _WIN32

#else
int POSIX_logicalToPhysical(int signal)
{
    switch (signal)
    {
    case SignalHandler::SIG_INT: return SIGINT;
    case SignalHandler::SIG_TERM: return SIGTERM;
    // In case the client asks for a SIG_CLOSE handler, accept and
    // bind it to a SIGTERM. Anyway the signal will never be raised
    case SignalHandler::SIG_CLOSE: return SIGTERM;
    case SignalHandler::SIG_RELOAD: return SIGHUP;
    default: 
        return -1; // SIG_ERR = -1
    }
}
#endif //_WIN32


#ifdef _WIN32
#else
int POSIX_physicalToLogical(int signal)
{
    switch (signal)
    {
    case SIGINT: return SignalHandler::SIG_INT;
    case SIGTERM: return SignalHandler::SIG_TERM;
    case SIGHUP: return SignalHandler::SIG_RELOAD;
    default:
        return SignalHandler::SIG_UNHANDLED;
    }
}
#endif //_WIN32

#ifdef _WIN32
#else
void POSIX_handleFunc(int signal)
{
    if (g_handler)
    {
        int signo = POSIX_physicalToLogical(signal);
        g_handler->handleSignal(signo);
    }
}
#endif //_WIN32

class Application : public SignalHandler
{
public:
    Application() : SignalHandler(SignalHandler::SIG_INT), w(nullptr) {}

    ~Application() { delete w; }

    int main(int argc, char *argv[]) {
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

        QApplication a(argc, argv);

        QCoreApplication::setOrganizationName("zecwallet-org");
        QCoreApplication::setApplicationName("zecwallet");

        QString locale = QLocale::system().name();
        locale.truncate(locale.lastIndexOf('_'));   // Get the language code
        qDebug() << "Loading locale " << locale;
        
        QTranslator translator;
        translator.load(QString(":/translations/res/zecwallet_") + locale);
        a.installTranslator(&translator);

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

        // Set up libsodium
        if (sodium_init() < 0) {
            /* panic! the library couldn't be initialized, it is not safe to use */
            qDebug() << "libsodium is not initialized!";
            exit(0);
        }

        // Command line parser
        QCommandLineParser parser;
        parser.setApplicationDescription("Shielded desktop wallet and embedded full node for Zcash");
        parser.addHelpOption();

        // A boolean option for running it headless
        QCommandLineOption headlessOption(QStringList() << "headless", "Running it via GUI.");
        parser.addOption(headlessOption);

        QCommandLineOption noembeddedOption(QStringList() << "no-embedded", "Disable embedded zcashd");
        parser.addOption(noembeddedOption);

        parser.process(a);
        if (parser.isSet(noembeddedOption)) {
            Settings::getInstance()->setUseEmbedded(false);
        } else {
            Settings::getInstance()->setUseEmbedded(true);
        }

        w = new MainWindow();
        w->setWindowTitle("ZecWallet v" + QString(APP_VERSION));

        if (parser.isSet(headlessOption)) {
            Settings::getInstance()->setHeadless(true);
            a.setQuitOnLastWindowClosed(false);    
        } else {
            Settings::getInstance()->setHeadless(false);
            w->show();
        }

        return QApplication::exec();
    }

    void DispatchToMainThread(std::function<void()> callback)
    {
        // any thread
        QTimer* timer = new QTimer();
        timer->moveToThread(qApp->thread());
        timer->setSingleShot(true);
        QObject::connect(timer, &QTimer::timeout, [=]()
        {
            // main thread
            callback();
            timer->deleteLater();
        });
        QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection, Q_ARG(int, 0));
    }

    bool handleSignal(int signal)
    {
        std::cout << std::endl << "Interrupted with signal " << signal << std::endl;
        
        if (w && w->getRPC()) {            
            // Blocking call to closeEvent on the UI thread.
            DispatchToMainThread([=] { 
                w->doClose(); 
                QApplication::quit();
            });
        } else {
            QApplication::quit();
        }
        
        return true;
    }

private:
    MainWindow* w;
};

int main(int argc, char* argv[])
{
    Application app;
    return app.main(argc, argv);
}

