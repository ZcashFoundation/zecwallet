#!/bin/bash

if [ -z $APP_VERSION ]; then echo "APP_VERSION is not set"; exit 1; fi
if [ -z $MXE_PATH ]; then echo "MXE_PATH is not set. Set it to ~/github/mxe/usr/bin"; exit 1; fi

export PATH=$MXE_PATH:$PATH

echo -n "Configuring..."
make distclean  > /dev/null
rm -f zec-qt-wallet-mingw.pro
rm -rf release/
#Mingw seems to have trouble with precompiled heades, so strip that option from the .pro file
cat zec-qt-wallet.pro | sed "s/precompile_header/release/g" | sed "s/PRECOMPILED_HEADER.*//g" > zec-qt-wallet-mingw.pro
echo "[OK]"


echo -n "Building......"
x86_64-w64-mingw32.static-qmake-qt5 zec-qt-wallet-mingw.pro CONFIG+=release > /dev/null
make -j32 > /dev/null
echo "[OK]"


echo -n "Packaging....."
mkdir release/zec-qt-wallet-v$APP_VERSION 
cp release/zec-qt-wallet.exe release/zec-qt-wallet-v$APP_VERSION 
cp README.md release/zec-qt-wallet-v$APP_VERSION 
cp LICENSE release/zec-qt-wallet-v$APP_VERSION 
cd release && zip -r Windows-zec-qt-wallet-v$APP_VERSION.zip zec-qt-wallet-v$APP_VERSION/ > /dev/null
cd ..
cp release/Windows-zec-qt-wallet-v$APP_VERSION.zip .


if [ -f Windows-zec-qt-wallet-v$APP_VERSION.zip ] ; then
    echo "[OK]"

    echo "Done. Build is Windows-zec-qt-wallet-v$APP_VERSION.zip"
    echo "Package contents:"
    unzip -l "Windows-zec-qt-wallet-v$APP_VERSION.zip"
else
    echo "[ERROR]"
    exit 1
fi
