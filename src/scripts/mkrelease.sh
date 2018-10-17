#!/bin/bash

if [ -z $QT_STATIC ]; then echo "QT_STATIC is not set"; exit 1; fi
if [ -z $APP_VERSION ]; then echo "APP_VERSION is not set"; exit 1; fi
if [ -z $PREV_VERSION ]; then echo "PREV_VERSION is not set"; exit 1; fi

echo "Updating version numbers"
# Replace the version number in the .pro file so it gets picked up everywhere
sed -i "s/${PREV_VERSION}/${APP_VERSION}/g" zcash-qt-wallet.pro > /dev/null

# Also update it in the README.md
sed -i "s/${PREV_VERSION}/${APP_VERSION}/g" README.md > /dev/null

echo "Configuring"
make distclean > /dev/null
$QT_STATIC/bin/qmake zcash-qt-wallet.pro -spec linux-clang CONFIG+=release > /dev/null

echo "Building"
rm -rf bin/zcash-qt-wallet* > /dev/null
make -j$(nproc) > /dev/null


# Test for QT
echo "Testing for no QT"

if [[ $(ldd zcash-qt-wallet | grep -i "QT") ]]; then
    echo "FOUND QT; ABORT"; 
    exit 1
else
    echo "No QT found"
fi

echo "Packaging"
mkdir bin/zcash-qt-wallet-v$APP_VERSION > /dev/null
cp zcash-qt-wallet bin/zcash-qt-wallet-v$APP_VERSION > /dev/null
cp README.md bin/zcash-qt-wallet-v$APP_VERSION > /dev/null
cp LICENSE bin/zcash-qt-wallet-v$APP_VERSION > /dev/null
cd bin && tar cvf zcash-qt-wallet-v$APP_VERSION.tar.gz zcash-qt-wallet-v$APP_VERSION/ > /dev/null

echo "Done"
