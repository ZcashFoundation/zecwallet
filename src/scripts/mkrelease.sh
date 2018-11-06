#!/bin/bash

if [ -z $QT_STATIC ]; then 
    echo "QT_STATIC is not set. Please set it to the base directory of a statically compiled Qt"; 
    exit 1; 
fi

if [ -z $MXE_PATH ]; then echo "MXE_PATH is not set. Set it to ~/github/mxe/usr/bin"; exit 1; fi
if [ -z $APP_VERSION ]; then echo "APP_VERSION is not set"; exit 1; fi
if [ -z $PREV_VERSION ]; then echo "PREV_VERSION is not set"; exit 1; fi

echo -n "Version files."
# Replace the version number in the .pro file so it gets picked up everywhere
sed -i "s/${PREV_VERSION}/${APP_VERSION}/g" zec-qt-wallet.pro > /dev/null

# Also update it in the README.md
sed -i "s/${PREV_VERSION}/${APP_VERSION}/g" README.md > /dev/null
echo "[OK]"

echo -n "Cleaning......"
rm -f bin/linux-zec-qt-wallet*
rm -rf release/
make distclean > /dev/null
echo "[OK]"

echo "Linux"

echo -n "Configuring..."
$QT_STATIC/bin/qmake zec-qt-wallet.pro -spec linux-clang CONFIG+=release > /dev/null
#Mingw seems to have trouble with precompiled headers, so strip that option from the .pro file
echo "[OK]"


echo -n "Building......"
rm -rf bin/zec-qt-wallet* > /dev/null
make -j$(nproc) > /dev/null
echo "[OK]"


# Test for Qt
echo -n "Static link..."
if [[ $(ldd zec-qt-wallet | grep -i "Qt") ]]; then
    echo "FOUND QT; ABORT"; 
    exit 1
fi
echo "[OK]"


echo -n "Packaging....."
mkdir bin/zec-qt-wallet-v$APP_VERSION > /dev/null
cp zec-qt-wallet bin/zec-qt-wallet-v$APP_VERSION > /dev/null
cp README.md bin/zec-qt-wallet-v$APP_VERSION > /dev/null
cp LICENSE bin/zec-qt-wallet-v$APP_VERSION > /dev/null
cd bin && tar cvf linux-zec-qt-wallet-v$APP_VERSION.tar.gz zec-qt-wallet-v$APP_VERSION/ > /dev/null
cd .. 
mkdir artifacts >/dev/null 2>&1
cp bin/linux-zec-qt-wallet-v$APP_VERSION.tar.gz ./artifacts

if [ -f artifacts/linux-zec-qt-wallet-v$APP_VERSION.tar.gz ] ; then
    echo "[OK]"

    echo "Done. Build is artifacts/linux-zec-qt-wallet-v$APP_VERSION.tar.gz"
    echo "Package contents:"
    tar tf "artifacts/linux-zec-qt-wallet-v$APP_VERSION.tar.gz"
else
    echo "[ERROR]"
    exit 1
fi

echo "Windows"

export PATH=$MXE_PATH:$PATH

echo -n "Configuring..."
make clean  > /dev/null
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
mkdir artifacts >/dev/null 2>&1
cp release/Windows-zec-qt-wallet-v$APP_VERSION.zip ./artifacts


if [ -f artifacts/Windows-zec-qt-wallet-v$APP_VERSION.zip ] ; then
    echo "[OK]"

    echo "Done. Build is artifacts/Windows-zec-qt-wallet-v$APP_VERSION.zip"
    echo "Package contents:"
    unzip -l "artifacts/Windows-zec-qt-wallet-v$APP_VERSION.zip"
else
    echo "[ERROR]"
    exit 1
fi
