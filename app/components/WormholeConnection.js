import React, { PureComponent } from 'react';
import QRCode from 'qrcode.react';
import dateformat from 'dateformat';
import cstyles from './Common.css';
import styles from './WormholeConnection.css';
import CompanionAppListener from '../companion';
import { ConnectedCompanionApp } from './AppState';

type Props = {
  companionAppListener: CompanionAppListener,
  connectedCompanionApp: ConnectedCompanionApp | null
};

type State = {
  tempKeyHex: string
};

export default class WormholeConnection extends PureComponent<Props, State> {
  constructor(props) {
    super(props);
    this.state = { tempKeyHex: null };
  }

  componentDidMount() {
    // If there is no temp key, create one
    const { companionAppListener } = this.props;
    const { tempKeyHex } = this.state;

    if (!tempKeyHex) {
      const newKey = companionAppListener.genNewKeyHex();
      companionAppListener.createTmpClient(newKey);
      this.setState({ tempKeyHex: newKey });
    }
  }

  componentWillUnmount() {
    const { companionAppListener } = this.props;
    companionAppListener.closeTmpClient();
  }

  disconnectCurrentMobile = () => {
    const { companionAppListener } = this.props;
    companionAppListener.disconnectLastClient();
  };

  render() {
    const { tempKeyHex } = this.state;
    const { connectedCompanionApp } = this.props;

    const clientName = (connectedCompanionApp && connectedCompanionApp.name) || null;
    const lastSeen = (connectedCompanionApp && connectedCompanionApp.lastSeen) || null;

    let datePart = null;
    let timePart = null;

    if (lastSeen) {
      const txDate = new Date(lastSeen);
      datePart = dateformat(txDate, 'mmm dd, yyyy');
      timePart = dateformat(txDate, 'hh:MM tt');
    }

    const connStr = `ws://127.0.0.1:7070,${tempKeyHex},1`;

    return (
      <div>
        <div className={[cstyles.xlarge, cstyles.padall, cstyles.center].join(' ')}>Connect Mobile App</div>

        <div className={styles.qrcodecontainer}>
          <div>This is your connection code. Scan this QR code from the Zecwallet Companion App.</div>

          <div className={[cstyles.center, cstyles.margintoplarge].join(' ')}>
            <QRCode value={connStr} size={256} className={styles.wormholeqr} />
          </div>
          <div className={[cstyles.sublight, cstyles.margintoplarge, cstyles.small].join(' ')}>{connStr}</div>
        </div>

        <div className={styles.appinfocontainer}>
          <div className={styles.appinfo}>
            {clientName && (
              <div>
                <div className={cstyles.flexspacebetween}>
                  <div style={{ flex: 1 }} className={cstyles.sublight}>
                    Current App Connected:
                  </div>
                  <div style={{ flex: 1 }}>{clientName}</div>
                </div>

                <div style={{ display: 'flex' }}>
                  <div style={{ flex: 1 }} className={cstyles.sublight}>
                    Last Seen:
                  </div>
                  <div style={{ flex: 1 }}>
                    {datePart} {timePart}
                  </div>
                </div>
                <div className={cstyles.margintoplarge}>
                  <button type="button" className={cstyles.primarybutton} onClick={this.disconnectCurrentMobile}>
                    Disconnect
                  </button>
                </div>
              </div>
            )}

            {!clientName && <div>No Companion App Connected</div>}
          </div>
        </div>
      </div>
    );
  }
}
