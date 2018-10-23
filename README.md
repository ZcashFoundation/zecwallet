zcash-qt-wallet is a z-Addr first, Sapling compatible wallet for zcashd that runs on Windows and Linux

![Screenshot](docs/screenshot-main.png?raw=true)

# Installation

Head over to the releases page and grab the latest binary. https://github.com/adityapk00/zcash-qt-wallet/releases


### Linux
Extract and run the binary
```
tar -xvf zcash-qt-wallet-v0.2.0.tar.gz
./zcash-qt-wallet-v0.2.0/zcash-qt-wallet
```

### Windows
Unzip the release binary and double click on zcash-qt-wallet to start.

## Prerequisites: zcashd
zcash-qt-wallet needs a zcash node running zcashd. Download the zcash node software 
from https://z.cash/download/ and start zcashd. If you are running Windows, you can 
get [WinZEC Command Line daemon](https://zcash.dl.mercerweiss.com/zcash-win-v2.0.1.zip) or run zcashd inside [WSL](https://docs.microsoft.com/en-us/windows/wsl/install-win10).

zcashd needs to run with RPC enabled and with a RPC username/password set. Add the following entries into ~/.zcash/zcash.conf

```
rpcuser=username
rpcpassword=password
```
zcash-qt-wallet should auto-detect zcashd and WinZEC. If you are running zcashd on WSL, then please set the connection parameters in the File->Settings menu. 


## Compiling from source
zcash-qt-wallet is written in C++ 14, and can be compiled with g++/clang++/visual c++. It also depends on Qt5, which you can get from here: https://www.qt.io/download

### Compiling on Linux

```
git clone https://github.com/adityapk00/zcash-qt-wallet.git
cd zcash-qt-wallet
/path/to/qt5/bin/qmake zcash-qt-wallet.pro CONFIG+=debug
make -j$(nproc)

./zcash-qt-wallet
```

### Compiling on Windows
You need Visual Studio 2017 (The free C++ Community Edition works just fine). 

From the VS Tools command prompt
```
c:\Qt5\bin\qmake.exe zcash-qt-wallet.pro CONFIG+=debug
nmake

debug\zcash-qt-wallet.exe
```

To create the Visual Studio project files so you can compile and run from Visual Studio:
```
c:\Qt5\bin\qmake.exe zcash-qt-wallet.pro -tp vc CONFIG+=debug
```

## Troubleshooting FAQ
### 1. "Connection Error"

Normally, zcash-qt-wallet can pick up the rpcuser/rpcpassword from zcash.conf. If it doesn't for some reason, you can set the username/password in the File->Settings menu. 
If you are connecting to a remote node, make sure that zcashd on the remote machine is accepting connections from your machine. The target machine's firewall needs to allow connections
from your host and also zcashd is set to be configured to accept connections from this host. 

The easiest way to connect to a remote node is probably to ssh to it with port forwarding like this:
```
ssh -L8232:127.0.0.1:8232 user@remotehost
```
### 2. "Not enough balance" when sending transactions
The most likely cause for this is that you are trying to spend unconfirmed funds. Unlike bitcoin, the zcash protocol doesn't let you spent unconfirmed funds yet. Please wait for 
1-2 blocks for the funds to confirm and retry the transaction. 

PS: zcash-qt-wallet is NOT an official wallet, and is not affiliated with the ZCash Company in any way. 