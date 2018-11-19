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
echo -n "Cleaning............"
make distclean >/dev/null 2>&1
rm -f artifacts/zec-qt-wallet-v$APP_VERSION.dmg
echo "[OK]"


echo -n "Building............"
# Build
$QT_PATH/bin/qmake zec-qt-wallet.pro CONFIG+=release >/dev/null
make -j4 >/dev/null
echo "[OK]"

#Qt deploy
echo -n "Deploying..........."
mkdir artifacts >/dev/null 2>&1
rm -f artifcats/zec-qt-wallet.dmg >/dev/null 2>&1
rm -f artifacts/rw* >/dev/null 2>&1
cp ../zcash/src/zcashd zec-qt-wallet.app/Contents/MacOS/
$QT_PATH/bin/macdeployqt zec-qt-wallet.app 
echo "[OK]"


echo -n "Building dmg........"
# create-dmg --volname "zec-qt-wallet-v$APP_VERSION" --volicon "res/logo.icns" --window-pos 200 120 --icon "zec-qt-wallet.app" 200 190  --app-drop-link 600 185 --hide-extension "zec-qt-wallet.app"  --window-size 800 400 --hdiutil-quiet --background res/dmgbg.png  artifacts/zec-qt-wallet.dmg zec-qt-wallet.app >/dev/null
mkdir bin/dmgbuild >/dev/null 2>&1
sed "s/RELEASE_VERSION/${APP_VERSION}/g" res/appdmg.json > bin/dmgbuild/appdmg.json
cp res/logo.icns bin/dmgbuild/
cp res/dmgbg.png bin/dmgbuild/

cp -r zec-qt-wallet.app bin/dmgbuild/

appdmg --quiet bin/dmgbuild/appdmg.json artifacts/zec-qt-wallet-v$APP_VERSION.dmg >/dev/null
if [ ! -f artifacts/zec-qt-wallet-v$APP_VERSION.dmg ]; then
    echo "[ERROR]"
    exit 1
fi
echo  "[OK]"
