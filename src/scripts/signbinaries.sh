#!/bin/bash

if [ -z $APP_VERSION ]; then echo "APP_VERSION is not set"; exit 1; fi

# Store the hash and signatures here
rm -rf release/signatures
mkdir -p release/signatures 

cd artifacts
echo "[Signing Binaries]"

# sha256sum the binaries
gsha256sum *$APP_VERSION* > ../release/signatures/sha256sum-v$APP_VERSION.txt

for i in $( ls *zec-qt-wallet-v$APP_VERSION*); do
  echo "Signing" $i
  gpg --batch --output ../release/signatures/$i.sig --detach-sig $i 
done

cp ../res/SIGNATURES_README ../release/signatures/README

cd ../release/signatures
tar -czf signatures-v$APP_VERSION.tar.gz *
cp signatures-v$APP_VERSION.tar.gz ../../artifacts

