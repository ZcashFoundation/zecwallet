/* eslint-disable max-classes-per-file */
import React, { Component } from "react";
import { ChildProcessWithoutNullStreams } from "child_process";
import routes from "../constants/routes.json";
import { RPCConfig, Info } from "./AppState";
import RPC from "../rpc";
import cstyles from "./Common.module.css";
import styles from "./LoadingScreen.module.css";
import Logo from "../assets/img/logobig.png";
import zcashdlogo from "../assets/img/zcashdlogo.gif";
import { Navigate } from "react-router-dom";

type Props = {
  setRPCConfig: (rpcConfig?: RPCConfig) => void;
  setInfo: (info: Info) => void;
};

class LoadingScreenState {
  creatingZcashConf: boolean;
  connectOverTor: boolean;
  enableFastSync: boolean;
  currentStatus: string | React.ReactElement;
  loadingDone: boolean;
  rpcConfig?: RPCConfig;
  zcashdSpawned: number;
  getinfoRetryCount: number;

  constructor() {
    this.currentStatus = "Loading...";
    this.creatingZcashConf = false;
    this.loadingDone = false;
    this.zcashdSpawned = 0;
    this.getinfoRetryCount = 0;
    this.rpcConfig = undefined;
    this.connectOverTor = false;
    this.enableFastSync = true;
  }
}

class LoadingScreen extends Component<Props, LoadingScreenState> {
  constructor(props: Props) {
    super(props);

    this.state = new LoadingScreenState();
  }

  componentDidMount() {
    (async () => {
      await this.loadZcashConf(true);
    })();
  }

  async loadZcashConf(createIfMissing: boolean) {
    // Load the RPC config from zcash.conf file
    const rpcConfig = await window.zecwallet.rpcconfig();

    if (!rpcConfig.username || !rpcConfig.password) {
      this.setState({
        currentStatus: (
          <div>
            <p>
              Your zcash.conf is missing a &quot;rpcuser&quot; or
              &quot;rpcpassword&quot;.
            </p>
            <p>
              Please add a &quot;rpcuser=some_username&quot; and
              &quot;rpcpassword=some_password&quot; to your zcash.conf to enable
              RPC access
            </p>
            {/* <p>Your zcash.conf is located at {zcashLocation}</p> */}
          </div>
        ),
      });
      return;
    }

    this.setState({ rpcConfig });

    // And setup the next getinfo
    this.setupNextGetInfo();
  }

  zcashd: ChildProcessWithoutNullStreams | null = null;

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
      const errString: string = (err as any).toString();
      const noConnection = errString.indexOf("NO_CONNECTION") > 0;

      if (noConnection) {
        // Try to start zcashd
        // this.startZcashd();
        // this.setupNextGetInfo();
        console.log("Cannot start zcashd because it is not supported yet");
        this.setState({
          currentStatus: (
            <div>
              {errString}
              <br />
              Please make sure zcashd is running.
            </div>
          ),
        });
      }

      if (errString.indexOf('"code":-28') > 0) {
        this.setState({ currentStatus: "Waiting for zcashd to start..." });
        const inc = getinfoRetryCount + 1;
        this.setState({ getinfoRetryCount: inc });
      } else if (errString.indexOf('"code":-8') > 0) {
        this.setState({
          currentStatus:
            "No accounts. Please run 'zcashd-wallet-tool' to create your wallet first...",
        });
      } else {
        // Generic error.
        this.setState({ currentStatus: errString });
      }

      if (noConnection && zcashdSpawned && getinfoRetryCount >= 10) {
        // Give up
        this.setState({
          currentStatus: (
            <span>
              Failed to start zcashd. Giving up! Please look at the debug.log
              file.
              <br />
              {/* <span
                className={cstyles.highlight}
              >{`${locateZcashConfDir()}/debug.log`}</span> */}
              <br />
              Please file an issue with Zecwallet
            </span>
          ),
        });
      }

      if (!noConnection) {
        this.setupNextGetInfo();
      }
    }
  }

  handleEnableFastSync = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.setState({ enableFastSync: event.target.checked });
  };

  handleTorEnabled = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.setState({ connectOverTor: event.target.checked });
  };

  render() {
    const {
      loadingDone,
      currentStatus,
      creatingZcashConf,
      connectOverTor,
      enableFastSync,
    } = this.state;

    // If still loading, show the status, else go to dashboard
    if (loadingDone) {
      return <Navigate to={routes.DASHBOARD} />;
    } else {
      return (
        <div className={[cstyles.center, styles.loadingcontainer].join(" ")}>
          {!creatingZcashConf && (
            <div className={cstyles.verticalflex}>
              <div style={{ marginTop: "100px" }}>
                <img src={Logo} width="200px;" alt="Logo" />
              </div>
              <div>{currentStatus}</div>
              <div></div>
            </div>
          )}

          {creatingZcashConf && (
            <div>
              <div className={cstyles.verticalflex}>
                <div
                  className={[
                    cstyles.verticalflex,
                    cstyles.center,
                    cstyles.margintoplarge,
                    cstyles.highlight,
                  ].join(" ")}
                >
                  <div className={[cstyles.xlarge].join(" ")}>
                    {" "}
                    Welcome To Zecwallet Fullnode!
                  </div>
                </div>

                <div
                  className={[cstyles.center, cstyles.margintoplarge].join(" ")}
                >
                  <img src={zcashdlogo} width="400px" alt="zcashdlogo" />
                </div>

                <div
                  className={[
                    cstyles.verticalflex,
                    cstyles.center,
                    cstyles.margintoplarge,
                  ].join(" ")}
                  style={{ width: "75%", marginLeft: "15%" }}
                >
                  <div>
                    Zecwallet Fullnode will download the{" "}
                    <span className={cstyles.highlight}>
                      entire Zcash Blockchain (~28GB)
                    </span>
                    , which might take several days to sync. If you want to get
                    started immediately, please consider{" "}
                    {/* eslint-disable-next-line jsx-a11y/anchor-is-valid */}
                    <a
                      className={cstyles.highlight}
                      style={{ textDecoration: "underline" }}
                      role="link"
                      onClick={() =>
                        window.zecwallet.openLink("https://www.zecwallet.co")
                      }
                    >
                      Zecwallet Lite
                    </a>
                    , which can get you started in under a minute.
                  </div>
                </div>

                <div
                  className={cstyles.left}
                  style={{ width: "75%", marginLeft: "15%" }}
                >
                  <div className={cstyles.margintoplarge} />
                  <div className={[cstyles.verticalflex].join(" ")}>
                    <div>
                      <input
                        type="checkbox"
                        onChange={this.handleTorEnabled}
                        defaultChecked={connectOverTor}
                      />
                      &nbsp; Connect over Tor
                    </div>
                    <div className={cstyles.sublight}>
                      Will connect over Tor. Please make sure you have the Tor
                      client installed and listening on port 9050.
                    </div>
                  </div>

                  <div className={cstyles.margintoplarge} />
                  <div className={[cstyles.verticalflex].join(" ")}>
                    <div>
                      <input
                        type="checkbox"
                        onChange={this.handleEnableFastSync}
                        defaultChecked={enableFastSync}
                      />
                      &nbsp; Enable Fast Sync
                    </div>
                    <div className={cstyles.sublight}>
                      When enabled, Zecwallet will skip some expensive
                      verifications of the zcashd blockchain when downloading.
                      This option is safe to use if you are creating a brand new
                      wallet.
                    </div>
                  </div>
                </div>

                <div className={cstyles.buttoncontainer}>
                  <button
                    type="button"
                    className={cstyles.primarybutton}
                    onClick={() =>
                      console.log(
                        "Cannot create zcash conf because it is not supported yet"
                      )
                    }
                  >
                    Start Zcash
                  </button>
                </div>
              </div>
            </div>
          )}
        </div>
      );
    }
  }
}

export default LoadingScreen;
