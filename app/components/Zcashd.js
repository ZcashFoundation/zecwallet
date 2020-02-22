/* eslint-disable react/prop-types */
import React, { Component } from 'react';
import { Info } from './AppState';
import cstyles from './Common.css';
import styles from './Zcashd.css';
import ScrollPane from './ScrollPane';
import Heart from '../assets/img/zcashdlogo.gif';

const DetailLine = ({ label, value }) => {
  return (
    <div className={styles.detailline}>
      <div className={cstyles.sublight}>{label} :</div>
      <div>{value}</div>
    </div>
  );
};

type Props = {
  info: Info,
  refresh: PropTypes.object.isRequired
};

export default class Zcashd extends Component<Props> {
  render() {
    const { info, refresh } = this.props;

    if (!info || !info.version) {
      return (
        <div>
          <div className={[cstyles.verticalflex, cstyles.center].join(' ')}>
            <div style={{ marginTop: '100px' }}>
              <i className={['fas', 'fa-times-circle'].join(' ')} style={{ fontSize: '96px', color: 'red' }} />
            </div>
            <div className={cstyles.margintoplarge}>Not Connected</div>
          </div>
        </div>
      );
      // eslint-disable-next-line no-else-return
    } else {
      let height = info.latestBlock;
      if (info.verificationProgress < 0.9999) {
        const progress = (info.verificationProgress * 100).toFixed(1);
        height = `${height} (${progress}%)`;
      }

      return (
        <div>
          <div className={styles.container}>
            <ScrollPane offsetHeight={0}>
              <div className={styles.imgcontainer}>
                <img src={Heart} alt="heart" />
              </div>

              <div className={styles.detailcontainer}>
                <div className={styles.detaillines}>
                  <DetailLine label="version" value={info.version} />
                  <DetailLine label="Network" value={info.testnet ? 'Testnet' : 'Mainnet'} />
                  <DetailLine label="Block Height" value={height} />
                  <DetailLine label="Connections" value={info.connections} />
                  <DetailLine label="Network Solution Rate" value={`${info.solps} Sol/s`} />
                </div>
              </div>

              <div className={cstyles.buttoncontainer}>
                <button className={cstyles.primarybutton} type="button" onClick={refresh}>
                  Refresh All Data
                </button>
              </div>

              <div className={cstyles.margintoplarge} />
            </ScrollPane>
          </div>
        </div>
      );
    }
  }
}
