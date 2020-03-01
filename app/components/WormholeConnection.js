import React, { PureComponent } from 'react';
import QRCode from 'qrcode.react';
import cstyles from './Common.css';
import styles from './WormholeConnection.css';
import CompanionAppListener from '../companion';

type Props = {
  companionAppListener: CompanionAppListener
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

  render() {
    const { companionAppListener } = this.props;
    const { tempKeyHex } = this.state;

    const connStr = `ws://127.0.0.1:7070,${tempKeyHex},1`;
    const clientName = companionAppListener.getLastClientName();

    return (
      <div>
        <div className={[cstyles.xlarge, cstyles.padall, cstyles.center].join(' ')}>Connect Mobile App</div>

        <div className={styles.qrcodecontainer}>
          <div>This is your connection code. Scan this QR code from the Zecwallet Companion App.</div>

          <div className={[cstyles.center, cstyles.margintoplarge].join(' ')}>
            <QRCode value={connStr} size={256} className={styles.wormholeqr} />
          </div>
          <div className={[cstyles.sublight, cstyles.margintoplarge].join(' ')}>{connStr}</div>
        </div>

        <div className={styles.appinfocontainer}>
          <div className={styles.appinfo}>
            <span className={cstyles.sublight}>Current App Connected:</span> {clientName}
          </div>
        </div>
      </div>
    );
  }
}
