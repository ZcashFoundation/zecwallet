safecoinwallet desktop wallet for Safecoin ($SAFE) that runs on Linux, Windows and macOS.

![Screenshots](safecoinwallet.png?raw=true)
# Installation

Head over to the releases page and grab the latest installers or binary. https://github.com/Fair-Exchange/safecoinwallet/releases

## safecoind
safecoinwallet needs a Safecoin full node running safecoind. If you already have a safecoind node running, safecoinwallet will connect to it. 

If you don't have one, safecoinwallet will start its embedded safecoind node. 

Additionally, if this is the first time you're running safecoinwallet or a safecoind daemon, safecoinwallet will download the zcash params (~1.7 GB) and configure `HUSH3.conf` for you. 

Pass `--no-embedded` to disable the embedded safecoind and force safecoinwallet to connect to an external node.

## Compiling from source
safecoinwallet is written in C++ 14, and can be compiled with g++/clang++/visual c++. It also depends on Qt5, which you can get from [here](https://www.qt.io/download). Note that if you are compiling from source, you won't get the embedded safecoind by default. You can either run an external safecoind, or compile safecoind as well. 


### Building on Linux

```
sudo apt-get install qt5-default qt5-qmake libqt5websockets5-dev
git clone https://github.com/Fair-Exchange/safecoinwallet.git
cd safecoinwallet
qmake safecoinwallet.pro CONFIG+=debug
make -j$(nproc)

./safecoinwallet
```

### Building on Windows
You need Visual Studio 2017 (The free C++ Community Edition works just fine). 

From the VS Tools command prompt
```
git clone  https://github.com/Fair-Exchange/safecoinwallet.git
cd safecoinwallet
c:\Qt5\bin\qmake.exe safecoinwallet.pro -spec win32-msvc CONFIG+=debug
nmake

debug\safecoinwallet.exe
```

To create the Visual Studio project files so you can compile and run from Visual Studio:
```
c:\Qt5\bin\qmake.exe safecoinwallet.pro -tp vc CONFIG+=debug
```

### Building on macOS
You need to install the Xcode app or the Xcode command line tools first, and then install Qt. 

```
git clone https://github.com/Fair-Exchange/safecoinwallet.git
cd safecoinwallet
qmake safecoinwallet.pro CONFIG+=debug
make

./safecoinwallet.app/Contents/MacOS/safecoinwallet
```

### Emulating the embedded node

In binary releases, safecoinwallet will use node binaries in the current directory to sync a node from scratch.
It does not attempt to download them, it bundles them. To simulate this from a developer setup, you can symlink
these four files in your Git repo:

```
    ln -s ../hush3/src/safecoind
    ln -s ../hush3/src/safecoin-cli
```

The above assumes safecoinwallet and hush3 git repos are in the same directory. File names on Windows will need to be tweaked.

### Support

For support or other questions, Join [Discord](https://discordapp.com/invite/vQgYGJz), or tweet at [@safecoins](https://twitter.com/safecoins) or [file an issue](https://github.com/Fair-Exchange/safecoinwallet/issues).

