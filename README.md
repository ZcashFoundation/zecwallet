SilentDragon desktop wallet for HUSH ($HUSH) that runs on Linux, Windows and macOS.

![Screenshots](silentdragon.png?raw=true)
# Installation

Head over to the releases page and grab the latest installers or binary. https://github.com/MyHush/SilentDragon/releases

## hushd
SilentDragon needs a Hush full node running hushd. If you already have a hushd node running, SilentDragon will connect to it. 

If you don't have one, SilentDragon will start its embedded hushd node. 

Additionally, if this is the first time you're running SilentDragon or a hushd daemon, SilentDragon will download the zcash params (~1.7 GB) and configure `HUSH3.conf` for you. 

Pass `--no-embedded` to disable the embedded hushd and force SilentDragon to connect to an external node.

## Compiling from source
SilentDragon is written in C++ 14, and can be compiled with g++/clang++/visual c++. It also depends on Qt5, which you can get from [here](https://www.qt.io/download). Note that if you are compiling from source, you won't get the embedded hushd by default. You can either run an external hushd, or compile hushd as well. 


### Building on Linux

```
sudo apt-get install qt5-default qt5-qmake libqt5websockets5-dev
git clone https://github.com/MyHush/SilentDragon.git
cd SilentDragon
qmake silentdragon.pro CONFIG+=debug
make -j$(nproc)

./silentdragon
```

### Building on Windows
You need Visual Studio 2017 (The free C++ Community Edition works just fine). 

From the VS Tools command prompt
```
git clone  https://github.com/MyHush/SilentDragon.git
cd SilentDragon
c:\Qt5\bin\qmake.exe silentdragon.pro -spec win32-msvc CONFIG+=debug
nmake

debug\SilentDragon.exe
```

To create the Visual Studio project files so you can compile and run from Visual Studio:
```
c:\Qt5\bin\qmake.exe silentdragon.pro -tp vc CONFIG+=debug
```

### Building on macOS
You need to install the Xcode app or the Xcode command line tools first, and then install Qt. 

```
git clone https://github.com/MyHush/SilentDragon.git
cd SilentDragon
qmake silentdragon.pro CONFIG+=debug
make

./SilentDragon.app/Contents/MacOS/SilentDragon
```

### Emulating the embedded node

In binary releases, SilentDragon will use node binaries in the current directory to sync a node from scratch.
It does not attempt to download them, it bundles them. To simulate this from a developer setup, you can symlink
these four files in your Git repo:

```
    ln -s ../hush3/src/hushd
    ln -s ../hush3/src/hush-cli
    ln -s ../hush3/src/komodod
    ln -s ../hush3/src/komodo-cli
```

The above assumes silentdragon and hush3 git repos are in the same directory. File names on Windows will need to be tweaked.

### Support

For support or other questions, Join [Discord](https://myhush.org/discord), or tweet at [@MyHushTeam](https://twitter.com/MyHushTeam) or [file an issue](https://github.com/MyHush/SilentDragon/issues).

