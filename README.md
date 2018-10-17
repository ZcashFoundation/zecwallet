zcash-qt-wallet is a z-Addr first wallet UI frontend for zcashd

![Screenshot](docs/screenshot-main.png?raw=true)

# Installation

zcash-qt-wallet needs a zcash node running zcashd. Download the zcash node software from https://z.cash/download/ and start zcashd.

## Prerequisites: zcashd
zcashd needs to run with RPC enabled and with a RPC username/password set. Add the following entries into ~/.zcash/zcash.conf

```
rpcuser=username
rpcpassword=password
```
and restart zcashd

## Installing zcash-qt-wallet
Head over to the releases page and grab the latest binary. https://github.com/adityapk00/zcash-qt-wallet/releases

Extract and run the binary
```
tar -xvf zcash-qt-wallet-v0.1.7.tar.gz
./zcash-qt-wallet-v0.1.7/zcash-qt-wallet
```

## Compiling from source
zcash-qt-wallet depends on Qt5, which you can get from here: https://www.qt.io/download

### Compiling on Linux
You need a C++14 compatible compiler like g++ or clang++

```
git clone https://github.com/adityapk00/zcash-qt-wallet.git
cd zcash-qt-wallet
/path/to/qt5/bin/qmake zcash-qt-wallet.pro CONFIG+=DEBUG
make -j$(nproc)
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