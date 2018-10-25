zec-qt-wallet is a z-Addr first, Sapling compatible wallet for zcashd that runs on Windows and Linux

![Screenshot](docs/screenshot-main.png?raw=true)

# Installation

Head over to the releases page and grab the latest binary. https://github.com/adityapk00/zec-qt-wallet/releases

### Linux
Extract and run the binary
```
tar -xvf zec-qt-wallet-v0.2.3.tar.gz
./zec-qt-wallet-v0.2.3/zec-qt-wallet
```

### Windows
Unzip the release binary and double click on zec-qt-wallet to start.

## Prerequisites: zcashd
zec-qt-wallet needs a Zcash node running zcashd. Linux users should download the zcash node software 
from [https://z.cash/download/](https://z.cash/download/), configure `zcash.conf`, download the parameters and start zcashd according to the [official documentation](https://zcash.readthedocs.io/en/latest/rtd_pages/user_guide.html). 

There is currently no official zcashd build for Windows so Windows users may either [cross-compile from source on Linux](https://zcash.readthedocs.io/en/latest/rtd_pages/user_guide.html#installation) to generate the necessary zcashd executables or simply download community hosted pre-compiled executables such as those hosted by WinZEC developer [@radix42](https://github.com/radix42) at https://zcash.dl.mercerweiss.com/zcash-win-v2.0.1.zip.

Alternitavely run zcashd inside [WSL](https://docs.microsoft.com/en-us/windows/wsl/install-win10).

For all installations zcashd needs to run with RPC enabled (`server=1`, which is the default) and with a RPC username/password set. Add the following entries into `~/.zcash/zcash.conf` for Linux or` C:\Users\your-username\AppData\Roaming\Zcash\zcash.conf` on Windows replacing the default values with a strong password. zec-qt-wallet should detect these settings but if that fails you may edit the connection settings manually via the `File->Settings` menu.

```
rpcuser=username
rpcpassword=password
```

Additionaly for Windows users the Zcash parameters must be manually downloaded and placed in `C:\Users\your-username\AppData\Roaming\ZcashParams`.  The following files are required (and are around ~1.7GB in total).

```
https://z.cash/downloads/sapling-spend.params
https://z.cash/downloads/sapling-output.params
https://z.cash/downloads/sprout-groth16.params
https://z.cash/downloads/sprout-proving.key
https://z.cash/downloads/sprout-verifying.key
```

If you are running zcashd on WSL, then please set the connection parameters in the `File->Settings` menu. 

## Compiling from source
zec-qt-wallet is written in C++ 14, and can be compiled with g++/clang++/visual c++. It also depends on Qt5, which you can get from here: https://www.qt.io/download

### Compiling on Linux

```
git clone https://github.com/adityapk00/zec-qt-wallet.git
cd zec-qt-wallet
/path/to/qt5/bin/qmake zec-qt-wallet.pro CONFIG+=debug
make -j$(nproc)

./zec-qt-wallet
```

### Compiling on Windows
You need Visual Studio 2017 (The free C++ Community Edition works just fine). 

From the VS Tools command prompt
```
c:\Qt5\bin\qmake.exe zec-qt-wallet.pro CONFIG+=debug
nmake

debug\zec-qt-wallet.exe
```

To create the Visual Studio project files so you can compile and run from Visual Studio:
```
c:\Qt5\bin\qmake.exe zec-qt-wallet.pro -tp vc CONFIG+=debug
```

## Troubleshooting FAQ
### 1. "Connection Error"

Normally, zec-qt-wallet can pick up the rpcuser/rpcpassword from zcash.conf. If it doesn't for some reason, you can set the username/password in the File->Settings menu. 
If you are connecting to a remote node, make sure that zcashd on the remote machine is accepting connections from your machine. The target machine's firewall needs to allow connections
from your host and also zcashd is set to be configured to accept connections from this host. 

The easiest way to connect to a remote node is probably to ssh to it with port forwarding like this:
```
ssh -L8232:127.0.0.1:8232 user@remotehost
```
### 2. "Not enough balance" when sending transactions
The most likely cause for this is that you are trying to spend unconfirmed funds. Unlike bitcoin, the zcash protocol doesn't let you spent unconfirmed funds yet. Please wait for 
1-2 blocks for the funds to confirm and retry the transaction. 

### Support or other questions
Tweet at [@zcashqtwallet](https://twitter.com/zcashqtwallet) for help.

_PS: zec-qt-wallet is NOT an official wallet, and is not affiliated with the Zerocoin Electric Coin Company in any way._