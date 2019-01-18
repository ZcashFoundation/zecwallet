#!/bin/bash

# First thing to do is see if libsodium.a exists in the res folder. If it does, then there's nothing to do
if [ -f res/libsodium.a ]; then
    exit 0
fi

echo "Building libsodium"

# Go into the lib sodium directory
cd res/libsodium
if [ ! -f libsodium-1.0.16.tar.gz ]; then
    wget https://download.libsodium.org/libsodium/releases/libsodium-1.0.16.tar.gz
fi

if [ ! -d libsodium-1.0.16 ]; then
    tar xf libsodium-1.0.16.tar.gz
fi

# Now build it
cd libsodium-1.0.16
bash configure
make -j$(nproc)
cd ..

# copy the library to the parents's res/ folder
cp libsodium-1.0.16/src/libsodium/.libs/libsodium.a ../
