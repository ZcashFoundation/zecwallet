/* eslint-disable jsx-a11y/interactive-supports-focus */
/* eslint-disable jsx-a11y/click-events-have-key-events */
/* eslint-disable max-classes-per-file */
import React, { Component } from 'react';
import { Redirect, withRouter } from 'react-router';
import ini from 'ini';
import fs from 'fs';
import request from 'request';
import progress from 'progress-stream';
import os from 'os';
import path from 'path';
import { remote, ipcRenderer, shell } from 'electron';
import { spawn } from 'child_process';
import { promisify } from 'util';
import routes from '../constants/routes.json';
import { RPCConfig, Info } from './AppState';
import RPC from '../rpc';
import cstyles from './Common.module.css';
import styles from './LoadingScreen.module.css';
import { NO_CONNECTION } from '../utils/utils';
import Logo from '../assets/img/logobig.png';
import zcashdlogo from '../assets/img/zcashdlogo.gif';

const locateZcashConfDir = () => {
  if (os.platform() === 'darwin') {
    return path.join(remote.app.getPath('appData'), 'Zcash');
  }

  if (os.platform() === 'linux') {
    return path.join(remote.app.getPath('home'), '.zcash');
  }

  return path.join(remote.app.getPath('appData'), 'Zcash');
};

const locateZcashConf = () => {
  if (os.platform() === 'darwin') {
    return path.join(remote.app.getPath('appData'), 'Zcash', 'zcash.conf');
  }

  if (os.platform() === 'linux') {
    return path.join(remote.app.getPath('home'), '.zcash', 'zcash.conf');
  }

  return path.join(remote.app.getPath('appData'), 'Zcash', 'zcash.conf');
};

const locateZcashd = () => {
  // const con = remote.getGlobal('console');
  // con.log(`App path = ${remote.app.getAppPath()}`);
  // con.log(`Unified = ${path.join(remote.app.getAppPath(), '..', 'bin', 'mac', 'zcashd')}`);

  if (os.platform() === 'darwin') {
    return path.join(remote.app.getAppPath(), '..', 'bin', 'mac', 'zcashd');
  }

  if (os.platform() === 'linux') {
    return path.join(remote.app.getAppPath(), '..', 'bin', 'linux', 'zcashd');
  }

  return path.join(remote.app.getAppPath(), '..', 'bin', 'win', 'zcashd.exe');
};

const locateZcashParamsDir = () => {
  if (os.platform() === 'darwin') {
    return path.join(remote.app.getPath('appData'), 'ZcashParams');
  }

  if (os.platform() === 'linux') {
    return path.join(remote.app.getPath('home'), '.zcash-params');
  }

  return path.join(remote.app.getPath('appData'), 'ZcashParams');
};

type Props = {
  setRPCConfig: (rpcConfig: RPCConfig) => void,
  setInfo: (info: Info) => void,
  history: PropTypes.object.isRequired
};

class LoadingScreenState {
  creatingZcashConf: boolean;

  connectOverTor: boolean;

  enableFastSync: boolean;

  currentStatus: string;

  loadingDone: boolean;

  rpcConfig: RPCConfig | null;

  zcashdSpawned: number;

  getinfoRetryCount: number;

  constructor() {
    this.currentStatus = 'Loading...';
    this.creatingZcashConf = false;
    this.loadingDone = false;
    this.zcashdSpawned = 0;
    this.getinfoRetryCount = 0;
    this.rpcConfig = null;
  }
}

class LoadingScreen extends Component<Props, LoadingScreenState> {
  constructor(props: Props) {
    super(props);

    this.state = new LoadingScreenState();
  }

  componentDidMount() {
    (async () => {
      const success = await this.ensureZcashParams();
      if (success) {
        await this.loadZcashConf(true);
        await this.setupExitHandler();
      }
    })();
  }

  download = (url, dest, name, cb) => {
    const file = fs.createWriteStream(dest);
    const sendReq = request.get(url);

    // verify response code
    sendReq.on('response', response => {
      if (response.statusCode !== 200) {
        return cb(`Response status was ${response.statusCode}`);
      }

      const totalSize = (parseInt(response.headers['content-length'], 10) / 1024 / 1024).toFixed(0);

      const str = progress({ time: 1000 }, pgrs => {
        this.setState({
          currentStatus: `Downloading ${name}... (${(pgrs.transferred / 1024 / 1024).toFixed(0)} MB / ${totalSize} MB)`
        });
      });

      sendReq.pipe(str).pipe(file);
    });

    // close() is async, call cb after close completes
    file.on('finish', () => file.close(cb));

    // check for request errors
    sendReq.on('error', err => {
      fs.unlink(dest);
      return cb(err.message);
    });

    file.on('error', err => {
      // Handle errors
      fs.unlink(dest); // Delete the file async. (But we don't check the result)
      return cb(err.message);
    });
  };

  ensureZcashParams = async () => {
    // Check if the zcash params dir exists and if the params files are present
    const dir = locateZcashParamsDir();
    if (!fs.existsSync(dir)) {
      fs.mkdirSync(dir);
    }

    // Check for the params
    const params = [
      { name: 'sapling-output.params', url: 'https://params.zecwallet.co/params/sapling-output.params' },
      { name: 'sapling-spend.params', url: 'https://params.zecwallet.co/params/sapling-spend.params' },
      { name: 'sprout-groth16.params', url: 'https://params.zecwallet.co/params/sprout-groth16.params' }
    ];

    // eslint-disable-next-line no-plusplus
    for (let i = 0; i < params.length; i++) {
      const p = params[i];

      const fileName = path.join(dir, p.name);
      if (!fs.existsSync(fileName)) {
        // Download and save this file
        this.setState({ currentStatus: `Downloading ${p.name}...` });

        try {
          // eslint-disable-next-line no-await-in-loop
          await promisify(this.download)(p.url, fileName, p.name);
        } catch (err) {
          console.log(`error: ${err}`);
          this.setState({ currentStatus: `Error downloading ${p.name}. The error was: ${err}` });
          return false;
        }
      }
    }

    return true;
  };

  async loadZcashConf(createIfMissing: boolean) {
    // Load the RPC config from zcash.conf file
    const zcashLocation = locateZcashConf();
    let confValues;
    try {
      confValues = ini.parse(await fs.promises.readFile(zcashLocation, { encoding: 'utf-8' }));
    } catch (err) {
      if (createIfMissing) {
        this.setState({ creatingZcashConf: true });
        return;
      }

      this.setState({
        currentStatus: `Could not create zcash.conf at ${zcashLocation}. This is a bug, please file an issue with Zecwallet`
      });
      return;
    }

    // Get the username and password
    const rpcConfig = new RPCConfig();
    rpcConfig.username = confValues.rpcuser;
    rpcConfig.password = confValues.rpcpassword;

    if (!rpcConfig.username || !rpcConfig.password) {
      this.setState({
        currentStatus: (
          <div>
            <p>Your zcash.conf is missing a &quot;rpcuser&quot; or &quot;rpcpassword&quot;.</p>
            <p>
              Please add a &quot;rpcuser=some_username&quot; and &quot;rpcpassword=some_password&quot; to your
              zcash.conf to enable RPC access
            </p>
            <p>Your zcash.conf is located at {zcashLocation}</p>
          </div>
        )
      });
      return;
    }

    const isTestnet = (confValues.testnet && confValues.testnet === '1') || false;
    const server = confValues.rpcbind || '127.0.0.1';
    const port = confValues.rpcport || (isTestnet ? '18232' : '8232');
    rpcConfig.url = `http://${server}:${port}`;

    this.setState({ rpcConfig });

    // And setup the next getinfo
    this.setupNextGetInfo();
  }

  createZcashconf = async () => {
    const { connectOverTor, enableFastSync } = this.state;

    const dir = locateZcashConfDir();
    if (!fs.existsSync(dir)) {
      fs.mkdirSync(dir);
    }

    const zcashConfPath = await locateZcashConf();

    let confContent = '';
    confContent += 'server=1\n';
    confContent += 'rpcuser=zecwallet\n';
    confContent += `rpcpassword=${Math.random()
      .toString(36)
      .substring(2, 15)}\n`;

    if (connectOverTor) {
      confContent += 'proxy=127.0.0.1:9050\n';
    }

    if (enableFastSync) {
      confContent += 'ibdskiptxverification=1\n';
    }

    await fs.promises.writeFile(zcashConfPath, confContent);

    this.setState({ creatingZcashConf: false });
    this.loadZcashConf(false);
  };

  zcashd: ChildProcessWithoutNullStreams | null = null;

  setupExitHandler = () => {
    // App is quitting, exit zcashd as well
    ipcRenderer.on('appquitting', async () => {
      if (this.zcashd) {
        const { history } = this.props;
        const { rpcConfig } = this.state;

        this.setState({ currentStatus: 'Waiting for zcashd to exit...' });
        history.push(routes.LOADING);

        this.zcashd.on('close', () => {
          ipcRenderer.send('appquitdone');
        });
        this.zcashd.on('exit', () => {
          ipcRenderer.send('appquitdone');
        });

        console.log('Sending stop');
        setTimeout(() => {
          RPC.doRPC('stop', [], rpcConfig);
        });
      } else {
        // And reply that we're all done.
        ipcRenderer.send('appquitdone');
      }
    });
  };

  startZcashd = async () => {
    const { zcashdSpawned } = this.state;

    if (zcashdSpawned) {
      this.setState({ currentStatus: 'zcashd start failed' });
      return;
    }

    const program = locateZcashd();
    console.log(program);

    this.zcashd = spawn(program);

    this.setState({ zcashdSpawned: 1 });
    this.setState({ currentStatus: 'zcashd starting...' });

    this.zcashd.on('error', err => {
      console.log(`zcashd start error, giving up. Error: ${err}`);
      // Set that we tried to start zcashd, and failed
      this.setState({ zcashdSpawned: 1 });

      // No point retrying.
      this.setState({ getinfoRetryCount: 10 });
    });
  };

  setupNextGetInfo() {
    setTimeout(() => this.getInfo(), 1000);
  }

  async getInfo() {
    const { rpcConfig, zcashdSpawned, getinfoRetryCount } = this.state;

    // Try getting the info.
    try {
      const info = await RPC.getInfoObject(rpcConfig);
      console.log(info);

      const { setRPCConfig, setInfo } = this.props;

      setRPCConfig(rpcConfig);
      setInfo(info);

      // This will cause a redirect to the dashboard
      this.setState({ loadingDone: true });
    } catch (err) {
      // Not yet finished loading. So update the state, and setup the next refresh
      this.setState({ currentStatus: err });

      if (err === NO_CONNECTION && !zcashdSpawned) {
        // Try to start zcashd
        this.startZcashd();
        this.setupNextGetInfo();
      }

      if (err === NO_CONNECTION && zcashdSpawned && getinfoRetryCount < 10) {
        this.setState({ currentStatus: 'Waiting for zcashd to start...' });
        const inc = getinfoRetryCount + 1;
        this.setState({ getinfoRetryCount: inc });
        this.setupNextGetInfo();
      }

      if (err === NO_CONNECTION && zcashdSpawned && getinfoRetryCount >= 10) {
        // Give up
        this.setState({
          currentStatus: (
            <span>
              Failed to start zcashd. Giving up! Please look at the debug.log file.
              <br />
              <span className={cstyles.highlight}>{`${locateZcashConfDir()}/debug.log`}</span>
              <br />
              Please file an issue with Zecwallet
            </span>
          )
        });
      }

      if (err !== NO_CONNECTION) {
        this.setupNextGetInfo();
      }
    }
  }

  handleEnableFastSync = event => {
    this.setState({ enableFastSync: event.target.checked });
  };

  handleTorEnabled = event => {
    this.setState({ connectOverTor: event.target.checked });
  };

  render() {
    const { loadingDone, currentStatus, creatingZcashConf, connectOverTor, enableFastSync } = this.state;

    // If still loading, show the status
    if (!loadingDone) {
      return (
        <div className={[cstyles.center, styles.loadingcontainer].join(' ')}>
          {!creatingZcashConf && (
            <div className={cstyles.verticalflex}>
              <div style={{ marginTop: '100px' }}>
                <img src={Logo} width="200px;" alt="Logo" />
              </div>
              <div>{currentStatus}</div>
            </div>
          )}

          {creatingZcashConf && (
            <div>
              <div className={cstyles.verticalflex}>
                <div
                  className={[cstyles.verticalflex, cstyles.center, cstyles.margintoplarge, cstyles.highlight].join(
                    ' '
                  )}
                >
                  <div className={[cstyles.xlarge].join(' ')}> Welcome To Zecwallet Fullnode!</div>
                </div>

                <div className={[cstyles.center, cstyles.margintoplarge].join(' ')}>
                  <img src={zcashdlogo} width="400px" alt="zcashdlogo" />
                </div>

                <div
                  className={[cstyles.verticalflex, cstyles.center, cstyles.margintoplarge].join(' ')}
                  style={{ width: '75%', marginLeft: '15%' }}
                >
                  <div>
                    Zecwallet Fullnode will download the{' '}
                    <span className={cstyles.highlight}>entire Zcash Blockchain (~28GB)</span>, which might take several
                    days to sync. If you want to get started immediately, please consider{' '}
                    <a
                      className={cstyles.highlight}
                      style={{ textDecoration: 'underline' }}
                      role="link"
                      onClick={() => shell.openExternal('https://www.zecwallet.co')}
                    >
                      Zecwallet Lite
                    </a>
                    , which can get you started in under a minute.
                  </div>
                </div>

                <div className={cstyles.left} style={{ width: '75%', marginLeft: '15%' }}>
                  <div className={cstyles.margintoplarge} />
                  <div className={[cstyles.verticalflex].join(' ')}>
                    <div>
                      <input type="checkbox" onChange={this.handleTorEnabled} defaultChecked={connectOverTor} />
                      &nbsp; Connect over Tor
                    </div>
                    <div className={cstyles.sublight}>
                      Will connect over Tor. Please make sure you have the Tor client installed and listening on port
                      9050.
                    </div>
                  </div>

                  <div className={cstyles.margintoplarge} />
                  <div className={[cstyles.verticalflex].join(' ')}>
                    <div>
                      <input type="checkbox" onChange={this.handleEnableFastSync} defaultChecked={enableFastSync} />
                      &nbsp; Enable Fast Sync
                    </div>
                    <div className={cstyles.sublight}>
                      When enabled, Zecwallet will skip some expensive verifications of the zcashd blockchain when
                      downloading. This option is safe to use if you are creating a brand new wallet.
                    </div>
                  </div>
                </div>

                <div className={cstyles.buttoncontainer}>
                  <button type="button" className={cstyles.primarybutton} onClick={this.createZcashconf}>
                    Start Zcash
                  </button>
                </div>
              </div>
            </div>
          )}
        </div>
      );
    }

    return <Redirect to={routes.DASHBOARD} />;
  }
}

export default withRouter(LoadingScreen);
