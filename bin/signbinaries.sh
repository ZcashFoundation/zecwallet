#!/bin/bash
# Accept the variables as command line arguments as well
POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -v|--version)
    APP_VERSION="$2"
    shift # past argument
    shift # past value
    ;;
    *)    # unknown option
    POSITIONAL+=("$1") # save it in an array for later
    shift # past argument
    ;;
esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

if [ -z $APP_VERSION ]; then echo "APP_VERSION is not set"; exit 1; fi

# Store the hash and signatures here
cd release
rm -rf signatures
mkdir signatures

# Remove previous signatures/hashes
rm -f sha256sum-$APP_VERSION.txt
rm -f signatures-$APP_VERSION.zip

# sha256sum the binaries
sha256sum Zecwallet*$APP_VERSION* > sha256sum-$APP_VERSION.txt

OIFS="$IFS"
IFS=$'\n'

for i in `find ./ -iname "Zecwallet*$APP_VERSION*" -o -iname "sha256sum-$APP_VERSION.txt"`; do
  echo "Signing" "$i"
  gpg --batch --output "signatures/$i.sig" --detach-sig "$i"
done

cp sha256sum-$APP_VERSION.txt signatures/
cp ../configs/SIGNATURES_README signatures/

zip -r signatures-$APP_VERSION.zip signatures/
