#-------------------------------------------------
#
# Project created by QtCreator 2018-10-05T09:54:45
#
#-------------------------------------------------

QT       += core gui network

CONFIG += precompile_header

PRECOMPILED_HEADER = precompiled.h

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = zcash-qt-wallet
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH  += 3rdparty/

RESOURCES     = application.qrc

MOC_DIR = bin
OBJECTS_DIR = bin

CONFIG += c++14

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    rpc.cpp \
    balancestablemodel.cpp \
    unspentoutput.cpp \
    3rdparty/qrcode/BitBuffer.cpp \
    3rdparty/qrcode/QrCode.cpp \
    3rdparty/qrcode/QrSegment.cpp \
    settings.cpp \
    sendtab.cpp \
    txtablemodel.cpp \
    transactionitem.cpp

HEADERS += \
    mainwindow.h \
    precompiled.h \
    rpc.h \
    balancestablemodel.h \
    unspentoutput.h \
    3rdparty/qrcode/BitBuffer.hpp \
    3rdparty/qrcode/QrCode.hpp \
    3rdparty/qrcode/QrSegment.hpp \
    3rdparty/json/json.hpp \
    settings.h \
    txtablemodel.h \
    transactionitem.h

FORMS += \
    mainwindow.ui \
    settings.ui \
    about.ui \
    confirm.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
