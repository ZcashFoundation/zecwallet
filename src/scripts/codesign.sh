#!/bin/bash

# Setup
bold=$(tput bold)
normal=$(tput sgr0)

# Print the usage and exit
do_help() {
    echo "codesign.sh v0.1";
    echo "";
    echo "Sign release binaries with gpg keysigning"
    echo "";
    echo "Usage:"
    echo "codesign.sh --version [version_id] file [file ...]"; 
    echo ""
    exit 1; 
}

# Print the usage for the version parameter and exit
do_version_missing() {
    echo "No release version identifier specified";
    echo "Please specify a release version with ${bold}--version${normal}"
    echo
    echo "Example:"
    echo "./codesign.sh --version 1.4 filename.msi"
    exit 1;
}

# Print the instructions for how to install dependencies
do_missing_command() {
    echo "Error: ${bold}$1${normal} was not installed"
    echo ""
    echo "One or more dependencies are missing. Please install all dependencies by running:"
    echo "${bold}brew install gsha256sum gnupg${normal}"
    exit 1;
}

# Print error message for missing private key
do_missing_gpg_key() {
    echo "Error: Couldn't find a local private key to sign with."
    echo
    echo "The command ${bold}gpg -K${normal} didn't return any keys. Did you forget to install the private keys on this machine?"
    exit 1;
}

# Accept the variables as command line arguments as well
POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -h|--help)
    do_help
    ;;
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

if [ -z $APP_VERSION ]; then
    do_version_missing
fi

# Check for existance of the gpg and sha256sum commands
hash gsha256sum 2>/dev/null || { 
    do_missing_command gsha256sum
    exit 1; 
}

hash gpg 2>/dev/null || { 
    do_missing_command gpg
    exit 1; 
}

hash zip 2>/dev/null || { 
    do_missing_command zip
    exit 1; 
}

# Check to see that we have a private key installed on this machine
if [[ -z $(gpg -K) ]]; then
    do_missing_gpg_key
fi

PackageContents=()

# Calculate the sha256sum for all input files
gsha256sum $@ > sha256sum.txt
PackageContents+=("sha256sum.txt")

# Sign all the files
for var in "$@"
do
    rm -f $var.sig
    echo "Signing" $var
    gpg --batch --output $var.sig --detach-sig $var
    PackageContents+=("$var.sig")
done

# Zip up everything into a neat package
ZipName=signatures-v$APP_VERSION.zip
echo "Zipping files into $ZipName"
rm -f $ZipName
zip $ZipName ${PackageContents[@]} 2>&1 >/dev/null

# Clean up intermediate files
rm ${PackageContents[@]}