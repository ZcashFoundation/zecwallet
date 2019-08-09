#!/bin/bash
if [ -z $QT_STATIC ]; then 
    echo "QT_STATIC is not set. Please set it to the base directory of a statically compiled Qt"; 
    exit 1; 
fi

if [ -z $APP_VERSION ]; then echo "APP_VERSION is not set"; exit 1; fi
if [ -z $PREV_VERSION ]; then echo "PREV_VERSION is not set"; exit 1; fi

if [ -z $ZCASH_DIR ]; then
    echo "ZCASH_DIR is not set. Please set it to the base directory of a Safecoin project with built Safecoin binaries."
    exit 1;
fi

if [ ! -f $ZCASH_DIR/artifacts/safecoind ]; then
    echo "Couldn't find safecoind in $ZCASH_DIR/artifacts/. Please build safecoind."
    exit 1;
fi

if [ ! -f $ZCASH_DIR/artifacts/safecoin-cli ]; then
    echo "Couldn't find safecoin-cli in $ZCASH_DIR/artifacts/. Please build safecoind."
    exit 1;
fi

# Ensure that safecoind is the right build
echo -n "safecoind version........."
if grep -q "zqwMagicBean" $ZCASH_DIR/artifacts/safecoind && ! readelf -s $ZCASH_DIR/artifacts/safecoind | grep -q "GLIBC_2\.25"; then 
    echo "[OK]"
else
    echo "[ERROR]"
    echo "safecoind doesn't seem to be a zqwMagicBean build or safecoind is built with libc 2.25"
    exit 1
fi

echo -n "safecoind.exe version....."
if grep -q "zqwMagicBean" $ZCASH_DIR/artifacts/safecoind.exe; then 
    echo "[OK]"
else
    echo "[ERROR]"
    echo "safecoind doesn't seem to be a zqwMagicBean build"
    exit 1
fi

echo -n "Version files.........."
# Replace the version number in the .pro file so it gets picked up everywhere
sed -i "s/${PREV_VERSION}/${APP_VERSION}/g" safecoinwallet.pro > /dev/null

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
$QT_STATIC/bin/qmake safecoinwallet.pro -spec linux-clang CONFIG+=release > /dev/null
echo "[OK]"


echo -n "Building..............."
rm -rf bin/safecoinwallet* > /dev/null
rm -rf bin/safecoinwallet* > /dev/null
make clean > /dev/null
make -j$(nproc) > /dev/null
echo "[OK]"


# Test for Qt
echo -n "Static link............"
if [[ $(ldd safecoinwallet | grep -i "Qt") ]]; then
    echo "FOUND QT; ABORT"; 
    exit 1
fi
echo "[OK]"


echo -n "Packaging.............."
mkdir bin/safecoinwallet-v$APP_VERSION > /dev/null
strip safecoinwallet

cp safecoinwallet                  bin/safecoinwallet-v$APP_VERSION > /dev/null
cp $ZCASH_DIR/artifacts/safecoind    bin/safecoinwallet-v$APP_VERSION > /dev/null
cp $ZCASH_DIR/artifacts/safecoin-cli bin/safecoinwallet-v$APP_VERSION > /dev/null
cp README.md                      bin/safecoinwallet-v$APP_VERSION > /dev/null
cp LICENSE                        bin/safecoinwallet-v$APP_VERSION > /dev/null

cd bin && tar czf linux-safecoinwallet-v$APP_VERSION.tar.gz safecoinwallet-v$APP_VERSION/ > /dev/null
cd .. 

mkdir artifacts >/dev/null 2>&1
cp bin/linux-safecoinwallet-v$APP_VERSION.tar.gz ./artifacts/linux-binaries-safecoinwallet-v$APP_VERSION.tar.gz
echo "[OK]"


if [ -f artifacts/linux-binaries-safecoinwallet-v$APP_VERSION.tar.gz ] ; then
    echo -n "Package contents......."
    # Test if the package is built OK
    if tar tf "artifacts/linux-binaries-safecoinwallet-v$APP_VERSION.tar.gz" | wc -l | grep -q "6"; then 
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
debdir=bin/deb/safecoinwallet-v$APP_VERSION
mkdir -p $debdir > /dev/null
mkdir    $debdir/DEBIAN
mkdir -p $debdir/usr/local/bin

cat src/scripts/control | sed "s/RELEASE_VERSION/$APP_VERSION/g" > $debdir/DEBIAN/control

cp safecoinwallet                   $debdir/usr/local/bin/
cp $ZCASH_DIR/artifacts/safecoind $debdir/usr/local/bin/safecoind

mkdir -p $debdir/usr/share/pixmaps/
cp res/zecwallet.xpm           $debdir/usr/share/pixmaps/

mkdir -p $debdir/usr/share/applications
cp src/scripts/desktopentry    $debdir/usr/share/applications/safecoinwallet.desktop

dpkg-deb --build $debdir >/dev/null
cp $debdir.deb                 artifacts/linux-deb-safecoinwallet-v$APP_VERSION.deb
echo "[OK]"



echo ""
echo "[Windows]"

if [ -z $MXE_PATH ]; then 
    echo "MXE_PATH is not set. Set it to ~/github/mxe/usr/bin if you want to build Windows"
    echo "Not building Windows"
    exit 0; 
fi

if [ ! -f $ZCASH_DIR/artifacts/safecoind.exe ]; then
    echo "Couldn't find safecoind.exe in $ZCASH_DIR/artifacts/. Please build safecoind.exe"
    exit 1;
fi


if [ ! -f $ZCASH_DIR/artifacts/safecoin-cli.exe ]; then
    echo "Couldn't find safecoin-cli.exe in $ZCASH_DIR/artifacts/. Please build safecoind.exe"
    exit 1;
fi

export PATH=$MXE_PATH:$PATH

echo -n "Configuring............"
make clean  > /dev/null
rm -f safecoinwallet-mingw.pro
rm -rf release/
#Mingw seems to have trouble with precompiled headers, so strip that option from the .pro file
cat safecoinwallet.pro | sed "s/precompile_header/release/g" | sed "s/PRECOMPILED_HEADER.*//g" > safecoinwallet-mingw.pro
echo "[OK]"


echo -n "Building..............."
x86_64-w64-mingw32.static-qmake-qt5 safecoinwallet-mingw.pro CONFIG+=release > /dev/null
make -j32 > /dev/null
echo "[OK]"


echo -n "Packaging.............."
mkdir release/safecoinwallet-v$APP_VERSION  
cp release/safecoinwallet.exe          release/safecoinwallet-v$APP_VERSION 
cp $ZCASH_DIR/artifacts/safecoind.exe    release/safecoinwallet-v$APP_VERSION > /dev/null
cp $ZCASH_DIR/artifacts/safecoin-cli.exe release/safecoinwallet-v$APP_VERSION > /dev/null
cp README.md                          release/safecoinwallet-v$APP_VERSION 
cp LICENSE                            release/safecoinwallet-v$APP_VERSION 
cd release && zip -r Windows-binaries-safecoinwallet-v$APP_VERSION.zip safecoinwallet-v$APP_VERSION/ > /dev/null
cd ..

mkdir artifacts >/dev/null 2>&1
cp release/Windows-binaries-safecoinwallet-v$APP_VERSION.zip ./artifacts/
echo "[OK]"

if [ -f artifacts/Windows-binaries-safecoinwallet-v$APP_VERSION.zip ] ; then
    echo -n "Package contents......."
    if unzip -l "artifacts/Windows-binaries-safecoinwallet-v$APP_VERSION.zip" | wc -l | grep -q "11"; then 
        echo "[OK]"
    else
        echo "[ERROR]"
        exit 1
    fi
else
    echo "[ERROR]"
    exit 1
fi
