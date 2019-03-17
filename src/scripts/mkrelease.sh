#!/bin/bash
if [ -z $QT_STATIC ]; then 
    echo "QT_STATIC is not set. Please set it to the base directory of a statically compiled Qt"; 
    exit 1; 
fi

if [ -z $APP_VERSION ]; then echo "APP_VERSION is not set"; exit 1; fi
if [ -z $PREV_VERSION ]; then echo "PREV_VERSION is not set"; exit 1; fi

if [ -z $ZCASH_DIR ]; then
    echo "ZCASH_DIR is not set. Please set it to the base directory of a Zcash project with built Zcash binaries."
    exit 1;
fi

if [ ! -f $ZCASH_DIR/artifacts/zcashd ]; then
    echo "Couldn't find zcashd in $ZCASH_DIR/artifacts/. Please build zcashd."
    exit 1;
fi

if [ ! -f $ZCASH_DIR/artifacts/zcash-cli ]; then
    echo "Couldn't find zcash-cli in $ZCASH_DIR/artifacts/. Please build zcashd."
    exit 1;
fi

# Ensure that zcashd is the right build
echo -n "zcashd version........."
if grep -q "zqwMagicBean" $ZCASH_DIR/artifacts/zcashd && ! readelf -s $ZCASH_DIR/artifacts/zcashd | grep -q "GLIBC_2\.25"; then 
    echo "[OK]"
else
    echo "[ERROR]"
    echo "zcashd doesn't seem to be a zqwMagicBean build or zcashd is built with libc 2.25"
    exit 1
fi

echo -n "zcashd.exe version....."
if grep -q "zqwMagicBean" $ZCASH_DIR/artifacts/zcashd.exe; then 
    echo "[OK]"
else
    echo "[ERROR]"
    echo "zcashd doesn't seem to be a zqwMagicBean build"
    exit 1
fi

echo -n "Version files.........."
# Replace the version number in the .pro file so it gets picked up everywhere
sed -i "s/${PREV_VERSION}/${APP_VERSION}/g" zec-qt-wallet.pro > /dev/null

# Also update it in the README.md
sed -i "s/${PREV_VERSION}/${APP_VERSION}/g" README.md > /dev/null
echo "[OK]"

echo -n "Cleaning..............."
rm -rf bin/*
rm -rf artifacts/*
make distclean >/dev/null 2>&1
echo "[OK]"

echo ""
echo "[Building on" `lsb_release -r`"]"

echo -n "Configuring............"
QT_STATIC=$QT_STATIC bash src/scripts/dotranslations.sh >/dev/null
$QT_STATIC/bin/qmake zec-qt-wallet.pro -spec linux-clang CONFIG+=release > /dev/null
echo "[OK]"


echo -n "Building..............."
rm -rf bin/zec-qt-wallet* > /dev/null
rm -rf bin/zecwallet* > /dev/null
make clean > /dev/null
make -j$(nproc) > /dev/null
echo "[OK]"


# Test for Qt
echo -n "Static link............"
if [[ $(ldd zecwallet | grep -i "Qt") ]]; then
    echo "FOUND QT; ABORT"; 
    exit 1
fi
echo "[OK]"


echo -n "Packaging.............."
mkdir bin/zecwallet-v$APP_VERSION > /dev/null
strip zecwallet

cp zecwallet                  bin/zecwallet-v$APP_VERSION > /dev/null
cp $ZCASH_DIR/artifacts/zcashd    bin/zecwallet-v$APP_VERSION > /dev/null
cp $ZCASH_DIR/artifacts/zcash-cli bin/zecwallet-v$APP_VERSION > /dev/null
cp README.md                      bin/zecwallet-v$APP_VERSION > /dev/null
cp LICENSE                        bin/zecwallet-v$APP_VERSION > /dev/null

cd bin && tar czf linux-zecwallet-v$APP_VERSION.tar.gz zecwallet-v$APP_VERSION/ > /dev/null
cd .. 

mkdir artifacts >/dev/null 2>&1
cp bin/linux-zecwallet-v$APP_VERSION.tar.gz ./artifacts/linux-binaries-zecwallet-v$APP_VERSION.tar.gz
echo "[OK]"


if [ -f artifacts/linux-binaries-zecwallet-v$APP_VERSION.tar.gz ] ; then
    echo -n "Package contents......."
    # Test if the package is built OK
    if tar tf "artifacts/linux-binaries-zecwallet-v$APP_VERSION.tar.gz" | wc -l | grep -q "6"; then 
        echo "[OK]"
    else
        echo "[ERROR]"
        exit 1
    fi    
else
    echo "[ERROR]"
    exit 1
fi

echo -n "Building deb..........."
debdir=bin/deb/zecwallet-v$APP_VERSION
mkdir -p $debdir > /dev/null
mkdir    $debdir/DEBIAN
mkdir -p $debdir/usr/local/bin

cat src/scripts/control | sed "s/RELEASE_VERSION/$APP_VERSION/g" > $debdir/DEBIAN/control

cp zecwallet                   $debdir/usr/local/bin/
cp $ZCASH_DIR/artifacts/zcashd $debdir/usr/local/bin/zqw-zcashd

mkdir -p $debdir/usr/share/pixmaps/
cp res/zecwallet.xpm           $debdir/usr/share/pixmaps/

mkdir -p $debdir/usr/share/applications
cp src/scripts/desktopentry    $debdir/usr/share/applications/zec-qt-wallet.desktop

dpkg-deb --build $debdir >/dev/null
cp $debdir.deb                 artifacts/linux-deb-zecwallet-v$APP_VERSION.deb
echo "[OK]"



echo ""
echo "[Windows]"

if [ -z $MXE_PATH ]; then 
    echo "MXE_PATH is not set. Set it to ~/github/mxe/usr/bin if you want to build Windows"
    echo "Not building Windows"
    exit 0; 
fi

if [ ! -f $ZCASH_DIR/artifacts/zcashd.exe ]; then
    echo "Couldn't find zcashd.exe in $ZCASH_DIR/artifacts/. Please build zcashd.exe"
    exit 1;
fi


if [ ! -f $ZCASH_DIR/artifacts/zcash-cli.exe ]; then
    echo "Couldn't find zcash-cli.exe in $ZCASH_DIR/artifacts/. Please build zcashd.exe"
    exit 1;
fi

export PATH=$MXE_PATH:$PATH

echo -n "Configuring............"
make clean  > /dev/null
rm -f zec-qt-wallet-mingw.pro
rm -rf release/
#Mingw seems to have trouble with precompiled headers, so strip that option from the .pro file
cat zec-qt-wallet.pro | sed "s/precompile_header/release/g" | sed "s/PRECOMPILED_HEADER.*//g" > zec-qt-wallet-mingw.pro
echo "[OK]"


echo -n "Building..............."
x86_64-w64-mingw32.static-qmake-qt5 zec-qt-wallet-mingw.pro CONFIG+=release > /dev/null
make -j32 > /dev/null
echo "[OK]"


echo -n "Packaging.............."
mkdir release/zecwallet-v$APP_VERSION  
cp release/zecwallet.exe          release/zecwallet-v$APP_VERSION 
cp $ZCASH_DIR/artifacts/zcashd.exe    release/zecwallet-v$APP_VERSION > /dev/null
cp $ZCASH_DIR/artifacts/zcash-cli.exe release/zecwallet-v$APP_VERSION > /dev/null
cp README.md                          release/zecwallet-v$APP_VERSION 
cp LICENSE                            release/zecwallet-v$APP_VERSION 
cd release && zip -r Windows-binaries-zecwallet-v$APP_VERSION.zip zecwallet-v$APP_VERSION/ > /dev/null
cd ..

mkdir artifacts >/dev/null 2>&1
cp release/Windows-binaries-zecwallet-v$APP_VERSION.zip ./artifacts/
echo "[OK]"

if [ -f artifacts/Windows-binaries-zecwallet-v$APP_VERSION.zip ] ; then
    echo -n "Package contents......."
    if unzip -l "artifacts/Windows-binaries-zecwallet-v$APP_VERSION.zip" | wc -l | grep -q "11"; then 
        echo "[OK]"
    else
        echo "[ERROR]"
        exit 1
    fi
else
    echo "[ERROR]"
    exit 1
fi
