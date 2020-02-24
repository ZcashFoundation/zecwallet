ZecWallet is a z-Addr first, Sapling compatible wallet and full node for zcashd that runs on Linux, Windows and macOS.

![Screenshot](resources/screenshot1.png?raw=true)
![Screenshots](resources/screenshot2.png?raw=true)

# Installation

Head over to the releases page and grab the latest installers or binary. https://github.com/ZcashFoundation/zecwallet/releases

### Linux

If you are on Debian/Ubuntu, please download the '.AppImage' package and just run it.

```
./Zecwallet.FullNode-0.9.2.AppImage
```

If you prefer to install a `.deb` package, that is also available.

```
sudo dpkg -i zecwallet_0.9.2_amd64.deb
sudo apt install -f
```

### Windows

Download and run the `.msi` installer and follow the prompts. Alternately, you can download the release binary, unzip it and double click on `zecwallet.exe` to start.

### macOS

Double-click on the `.dmg` file to open it, and drag `Zecwallet Fullnode` on to the Applications link to install.

## zcashd

ZecWallet needs a Zcash node running zcashd. If you already have a zcashd node running, ZecWallet will connect to it.

If you don't have one, ZecWallet will start its embedded zcashd node.

Additionally, if this is the first time you're running ZecWallet or a zcashd daemon, ZecWallet will download the Zcash params (~777 MB) and configure `zcash.conf` for you.

## Compiling from source

ZecWallet is written in Electron/Javascript and can be build from source. Note that if you are compiling from source, you won't get the embedded zcashd by default. You can either run an external zcashd, or compile zcashd as well.

#### Pre-Requisits

You need to have the following software installed before you can build Zecwallet Fullnode

- Nodejs v12.16.1 or higher - https://nodejs.org
- Yarn - https://yarnpkg.com

```
git clone https://github.com/ZcashFoundation/zecwallet.git
cd zecwallet

yarn install
yarn build
```

To start in development mode, run

```
yarn dev
```

To start in production mode, run

```
yarn start
```

### [Troubleshooting Guide & FAQ](https://github.com/ZcashFoundation/zecwallet/wiki/Troubleshooting-&-FAQ)

Please read the [troubleshooting guide](https://docs.zecwallet.co/troubleshooting/) for common problems and solutions.
For support or other questions, tweet at [@zecwallet](https://twitter.com/zecwallet) or [file an issue](https://github.com/ZcashFoundation/zecwallet/issues).

_PS: ZecWallet is NOT an official wallet, and is not affiliated with the Electric Coin Company in any way._
