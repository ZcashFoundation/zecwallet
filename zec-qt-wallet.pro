#-------------------------------------------------
#
# Project created by QtCreator 2018-10-05T09:54:45
#
#-------------------------------------------------

QT       += core gui network

CONFIG += precompile_header

PRECOMPILED_HEADER = src/precompiled.h

QT += widgets
QT += websockets

TARGET = zec-qt-wallet

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += \
    QT_DEPRECATED_WARNINGS

INCLUDEPATH  += src/3rdparty/

RESOURCES     = application.qrc

MOC_DIR = bin
OBJECTS_DIR = bin
UI_DIR = src

CONFIG += c++14

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/rpc.cpp \
    src/balancestablemodel.cpp \
    src/3rdparty/qrcode/BitBuffer.cpp \
    src/3rdparty/qrcode/QrCode.cpp \
    src/3rdparty/qrcode/QrSegment.cpp \
    src/settings.cpp \
    src/sendtab.cpp \
    src/senttxstore.cpp \
    src/txtablemodel.cpp \
    src/turnstile.cpp \
    src/qrcodelabel.cpp \
    src/connection.cpp \
    src/fillediconlabel.cpp \
    src/addressbook.cpp \
    src/logger.cpp \
    src/addresscombo.cpp \
    src/websockets.cpp \
    src/mobileappconnector.cpp \
    src/recurring.cpp

HEADERS += \
    src/mainwindow.h \
    src/precompiled.h \
    src/rpc.h \
    src/balancestablemodel.h \
    src/3rdparty/qrcode/BitBuffer.hpp \
    src/3rdparty/qrcode/QrCode.hpp \
    src/3rdparty/qrcode/QrSegment.hpp \
    src/3rdparty/json/json.hpp \
    src/settings.h \
    src/txtablemodel.h \
    src/senttxstore.h \
    src/turnstile.h \
    src/qrcodelabel.h \
    src/connection.h \
    src/fillediconlabel.h \
    src/addressbook.h \
    src/logger.h \
    src/addresscombo.h \ 
    src/websockets.h \
    src/mobileappconnector.h \
    src/recurring.h

FORMS += \
    src/mainwindow.ui \
    src/settings.ui \
    src/about.ui \
    src/confirm.ui \
    src/turnstile.ui \
    src/turnstileprogress.ui \
    src/privkey.ui \
    src/memodialog.ui \ 
    src/connection.ui \
    src/zboard.ui \
    src/addressbook.ui \
    src/mobileappconnector.ui \
    src/createzcashconfdialog.ui \
    src/recurringdialog.ui \
    src/newrecurring.ui


TRANSLATIONS = res/zec_qt_wallet_es.ts \
               res/zec_qt_wallet_fr.ts \
               res/zec_qt_wallet_de.ts \
               res/zec_qt_wallet_pt.ts \
               res/zec_qt_wallet_it.ts 

win32: RC_ICONS = res/icon.ico
ICON = res/logo.icns

libsodium.target = $$PWD/res/libsodium.a
libsodium.commands = res/libsodium/buildlibsodium.sh

QMAKE_EXTRA_TARGETS += libsodium
QMAKE_CLEAN += res/libsodium.a

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/res/ -llibsodium
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/res/ -llibsodiumd
else:unix: LIBS += -L$$PWD/res/ -lsodium

INCLUDEPATH += $$PWD/res
DEPENDPATH += $$PWD/res

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/res/liblibsodium.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/res/liblibsodium.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/res/libsodium.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/res/libsodiumd.lib
else:unix: PRE_TARGETDEPS += $$PWD/res/libsodium.a
