ZecWallet Fullnode is a z-Addr first, Sapling compatible wallet and full node for zcashd that runs on Linux, Windows and macOS.

![Screenshot](resources/screenshot1.png?raw=true)
![Screenshots](resources/screenshot2.png?raw=true)

# Installation

**Note**: Zecwallet Fullnode will download the **entire blockchain (about 36GB)**, and requires some familiarity with the command line. If you don't want to download the blockchain but prefer a Lite wallet, please check out [Zecwallet Lite](https://www.zecwallet.co).

Head over to the releases page and grab the latest installers or binary. https://www.zecwallet.co

### Linux

If you are on Debian/Ubuntu, please download the '.AppImage' package and just run it.

```
./Zecwallet.Fullnode-1.8.6.AppImage
```

If you prefer to install a `.deb` package, that is also available.

```
sudo apt install -f ./zecwallet_1.8.6_amd64.deb
```

### Windows

Download and run the `.msi` installer and follow the prompts. Alternately, you can download the release binary, unzip it and double click on `zecwallet.exe` to start.

### macOS

Double-click on the `.dmg` file to open it, and drag `Zecwallet Fullnode` on to the Applications link to install.

## Running zcashd

ZecWallet needs a Zcash node running zcashd. If you already have a zcashd node running, ZecWallet will connect to it.

You need to manually start the zcashd node so that Zecwallet can connect to it. The first time you run zcashd, you need to:

1. Create a zcash.conf file with at least:

```
server=1
rpcuser=<someusername>
rpcpassword=<somepassword>
```

2. Run `./fetch-params.sh` to get the Zcash params

3. Run `./zcashd-wallet-tool` to configure your seed phrase and set up your Orchard accounts.

All binaries needed are included in the distribution.

## Compiling from source

ZecWallet is written in Electron/Typescript and can be build from source. Note that if you are compiling from source, you won't get the embedded zcashd by default. You can either run an external zcashd, or compile zcashd as well.

#### Pre-Requisits

You need to have the following software installed before you can build Zecwallet Fullnode

- Nodejs v14 or higher - https://nodejs.org
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
