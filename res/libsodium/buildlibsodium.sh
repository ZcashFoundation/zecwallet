#!/bin/bash

# First thing to do is see if libsodium.a exists in the res folder. If it does, then there's nothing to do
if [ -f res/libsodium.a ]; then
    exit 0
fi

echo "Building libsodium"

# Go into the lib sodium directory
cd res/libsodium
if [ ! -f libsodium-1.0.16.tar.gz ]; then
    curl -LO https://download.libsodium.org/libsodium/releases/libsodium-1.0.16.tar.gz
fi

if [ ! -d libsodium-1.0.16 ]; then
    tar xf libsodium-1.0.16.tar.gz
fi

# Now build it
cd libsodium-1.0.16
LIBS="" ./configure
make clean
if [[ "$OSTYPE" == "darwin"* ]]; then
    make CFLAGS="-mmacosx-version-min=10.11" CPPFLAGS="-mmacosx-version-min=10.11" -j4
else
    make -j4
fi
cd ..

# copy the library to the parents's res/ folder
cp libsodium-1.0.16/src/libsodium/.libs/libsodium.a ../
