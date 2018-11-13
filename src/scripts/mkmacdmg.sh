#!/bin/bash

if [ -z $QT_PATH ]; then 
    echo "QT_PATH is not set. Please set it to the base directory of Qt"; 
    exit 1; 
fi

#Clean
make distclean 2>&1 >/dev/null

# Build
$QT_PATH/bin/qmake zec-qt-wallet.pro CONFIG+=release
make -j4 >/dev/null

#Qt deploy
mkdir artifacts
rm -f zec-qt-wallet.dmg
$QT_PATH/bin/macdeployqt zec-qt-wallet.app -dmg

# Finish
mv zec-qt-wallet.dmg artifacts/MacOS-zec-qt-wallet-v0.3.0.dmg
