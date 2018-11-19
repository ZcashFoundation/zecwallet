#!/bin/bash

if [ -z $QT_PATH ]; then 
    echo "QT_PATH is not set. Please set it to the base directory of Qt"; 
    exit 1; 
fi

if [ -z $APP_VERSION ]; then
    echo "APP_VERSION is not set. Please set it to the current release version of the app";
    exit 1;
fi

if [ ! -f ../zcash/src/zcashd ]; then
    echo "Could not find compiled zcashd in ../zcash/src/.";
    exit 1;
fi

#Clean
make distclean >/dev/null 2>&1
rm artifacts/zec-qt-wallet-v$APP_VERSION.dmg

# Build
$QT_PATH/bin/qmake zec-qt-wallet.pro CONFIG+=release
make -j4 >/dev/null

#Qt deploy
mkdir artifacts >/dev/null 2>&1
rm -f artifcats/zec-qt-wallet.dmg >/dev/null 2>&1
rm -f artifacts/rw* >/dev/null 2>&1
cp ../zcash/src/zcashd zec-qt-wallet.app/Contents/MacOS/
$QT_PATH/bin/macdeployqt zec-qt-wallet.app 

# create-dmg --volname "zec-qt-wallet-v$APP_VERSION" --volicon "res/logo.icns" --window-pos 200 120 --icon "zec-qt-wallet.app" 200 190  --app-drop-link 600 185 --hide-extension "zec-qt-wallet.app"  --window-size 800 400 --hdiutil-quiet --background res/dmgbg.png  artifacts/zec-qt-wallet.dmg zec-qt-wallet.app >/dev/null

appdmg res/appdmg.json artifacts/zec-qt-wallet-v$APP_VERSION.dmg
