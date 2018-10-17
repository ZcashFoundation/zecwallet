#!/bin/bash

if [ -z $QT_STATIC ]; then echo "QT_STATIC is not set"; exit 1; fi
if [ -z $APP_VERSION ]; then echo "APP_VERSION is not set"; exit 1; fi
if [ -z $PREV_VERSION ]; then echo "PREV_VERSION is not set"; exit 1; fi

echo -n "Version files."
# Replace the version number in the .pro file so it gets picked up everywhere
sed -i "s/${PREV_VERSION}/${APP_VERSION}/g" zcash-qt-wallet.pro > /dev/null

# Also update it in the README.md
sed -i "s/${PREV_VERSION}/${APP_VERSION}/g" README.md > /dev/null
echo "[OK]"


echo -n "Configuring..."
rm -f "bin/linux-zcash-qt-wallet-v$APP_VERSION.tar.gz"
make distclean > /dev/null
$QT_STATIC/bin/qmake zcash-qt-wallet.pro -spec linux-clang CONFIG+=release > /dev/null
echo "[OK]"


echo -n "Building......"
rm -rf bin/zcash-qt-wallet* > /dev/null
make -j$(nproc) > /dev/null
echo "[OK]"


# Test for Qt
echo -n "Static link..."
if [[ $(ldd zcash-qt-wallet | grep -i "Qt") ]]; then
    echo "FOUND QT; ABORT"; 
    exit 1
fi
echo "[OK]"


echo -n "Packaging....."
mkdir bin/zcash-qt-wallet-v$APP_VERSION > /dev/null
cp zcash-qt-wallet bin/zcash-qt-wallet-v$APP_VERSION > /dev/null
cp README.md bin/zcash-qt-wallet-v$APP_VERSION > /dev/null
cp LICENSE bin/zcash-qt-wallet-v$APP_VERSION > /dev/null
cd bin && tar cvf linux-zcash-qt-wallet-v$APP_VERSION.tar.gz zcash-qt-wallet-v$APP_VERSION/ > /dev/null
cd .. 

if [ -f bin/linux-zcash-qt-wallet-v$APP_VERSION.tar.gz ] ; then
    echo "[OK]"

    echo "Done. Build is bin/linux-zcash-qt-wallet-v$APP_VERSION.tar.gz"
    echo "Package contents:"
    tar tf "bin/linux-zcash-qt-wallet-v$APP_VERSION.tar.gz"
else
    echo "[ERROR]"
    exit 1
fi