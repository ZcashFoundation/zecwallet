/* eslint-disable react/prop-types */
/* eslint-disable jsx-a11y/click-events-have-key-events */
/* eslint-disable jsx-a11y/no-static-element-interactions */
import React, { Component } from 'react';
import Modal from 'react-modal';
import dateformat from 'dateformat';
import { BalanceBlockHighlight } from './Dashboard';
import styles from './Transactions.css';
import cstyles from './Common.css';
import { Transaction, Info } from './AppState';
import ScrollPane from './ScrollPane';
import Utils from '../utils/utils';
import AddressBook from './Addressbook';

const TxModal = ({ modalIsOpen, tx, closeModal, currencyName, zecPrice }) => {
  let txid = '';
  let type = '';
  let typeIcon = '';
  let typeColor = '';
  let confirmations = 0;
  let detailedTxns = [];
  let amount = 0;
  let datePart = '';
  let timePart = '';

  if (tx) {
    txid = tx.txid;
    type = tx.type;
    if (tx.type === 'receive') {
      typeIcon = 'fa-arrow-circle-down';
      typeColor = 'green';
    } else {
      typeIcon = 'fa-arrow-circle-up';
      typeColor = 'red';
    }

    datePart = dateformat(tx.time * 1000, 'mmm dd, yyyy');
    timePart = dateformat(tx.time * 1000, 'hh:MM tt');

    confirmations = tx.confirmations;
    detailedTxns = tx.detailedTxns;
    amount = Math.abs(tx.amount);
  }

  return (
    <Modal
      isOpen={modalIsOpen}
      onRequestClose={closeModal}
      className={styles.txmodal}
      overlayClassName={styles.txmodalOverlay}
    >
      <div className={[cstyles.verticalflex].join(' ')}>
        <div className={[cstyles.marginbottomlarge, cstyles.center].join(' ')}>Transaction Status</div>

        <div className={[cstyles.center].join(' ')}>
          <i className={['fas', typeIcon].join(' ')} style={{ fontSize: '96px', color: typeColor }} />
        </div>

        <div className={[cstyles.center].join(' ')}>
          {type}
          <BalanceBlockHighlight
            zecValue={amount}
            usdValue={Utils.getZecToUsdString(zecPrice, Math.abs(amount))}
            currencyName={currencyName}
          />
        </div>

        <div className={[cstyles.flexspacebetween].join(' ')}>
          <div>
            <div className={[cstyles.sublight].join(' ')}>Time</div>
            <div>
              {datePart} {timePart}
            </div>
          </div>
          <div>
            <div className={[cstyles.sublight].join(' ')}>Confirmations</div>
            <div>{confirmations}</div>
          </div>
        </div>

        <div className={cstyles.margintoplarge} />

        <div className={[cstyles.sublight].join(' ')}>TXID</div>
        <div>{txid}</div>

        <div className={cstyles.margintoplarge} />

        {detailedTxns.map(txdetail => {
          const { bigPart, smallPart } = Utils.splitZecAmountIntoBigSmall(Math.abs(txdetail.amount));

          let { address } = txdetail;
          const { memo } = txdetail;

          if (!address) {
            address = '(Shielded)';
          }

          return (
            <div key={address} className={cstyles.verticalflex}>
              <div className={[cstyles.sublight].join(' ')}>Address</div>
              <div>{Utils.splitStringIntoChunks(address, 6).join(' ')}</div>

              <div className={cstyles.margintoplarge} />

              <div className={[cstyles.sublight].join(' ')}>Amount</div>
              <div>
                <span>
                  {currencyName} {bigPart}
                </span>
                <span className={[cstyles.small, cstyles.zecsmallpart].join(' ')}>{smallPart}</span>
              </div>

              <div className={cstyles.margintoplarge} />

              {memo && (
                <div>
                  <div className={[cstyles.sublight].join(' ')}>Memo</div>
                  <div>{memo}</div>
                </div>
              )}
            </div>
          );
        })}

        <div className={[cstyles.center, cstyles.margintoplarge].join(' ')}>
          <button type="button" className={cstyles.primarybutton} onClick={closeModal}>
            Close
          </button>
        </div>
      </div>
    </Modal>
  );
};

const TxItemBlock = ({ transaction, currencyName, zecPrice, txClicked, addressBookMap }) => {
  const txDate = new Date(transaction.time * 1000);
  const datePart = dateformat(txDate, 'mmm dd, yyyy');
  const timePart = dateformat(txDate, 'hh:MM tt');

  return (
    <div>
      <div className={[cstyles.small, cstyles.sublight, styles.txdate].join(' ')}>{datePart}</div>
      <div
        className={[cstyles.well, styles.txbox].join(' ')}
        onClick={() => {
          txClicked(transaction);
        }}
      >
        <div className={styles.txtype}>
          <div>{transaction.type}</div>
          <div className={[cstyles.padtopsmall, cstyles.sublight].join(' ')}>{timePart}</div>
        </div>
        {transaction.detailedTxns.map(txdetail => {
          const { bigPart, smallPart } = Utils.splitZecAmountIntoBigSmall(Math.abs(txdetail.amount));

          let { address } = txdetail;
          const { memo } = txdetail;

          if (!address) {
            address = '(Shielded)';
          }

          const label = addressBookMap[address] || '';

          return (
            <div key={address} className={styles.txaddressamount}>
              <div className={styles.txaddress}>
                <div className={cstyles.highlight}>{label}</div>
                <div>{Utils.splitStringIntoChunks(address, 6).join(' ')}</div>
                <div className={[cstyles.small, cstyles.sublight, cstyles.padtopsmall, styles.txmemo].join(' ')}>
                  {memo}
                </div>
              </div>
              <div className={[styles.txamount, cstyles.right].join(' ')}>
                <div>
                  <span>
                    {currencyName} {bigPart}
                  </span>
                  <span className={[cstyles.small, cstyles.zecsmallpart].join(' ')}>{smallPart}</span>
                </div>
                <div className={[cstyles.sublight, cstyles.small, cstyles.padtopsmall].join(' ')}>
                  {Utils.getZecToUsdString(zecPrice, Math.abs(txdetail.amount))}
                </div>
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
};

type Props = {
  transactions: Transaction[],
  addressBook: AddressBook[],
  info: Info
};

type State = {
  clickedTx: Transaction | null,
  modalIsOpen: boolean
};

export default class Transactions extends Component<Props, State> {
  constructor(props: Props) {
    super(props);

    this.state = { clickedTx: null, modalIsOpen: false };
  }

  txClicked = (tx: Transaction) => {
    // Show the modal
    if (!tx) return;
    this.setState({ clickedTx: tx, modalIsOpen: true });
  };

  closeModal = () => {
    this.setState({ clickedTx: null, modalIsOpen: false });
  };

  render() {
    const { transactions, info, addressBook } = this.props;
    const { clickedTx, modalIsOpen } = this.state;

    const addressBookMap = addressBook.reduce((map, obj) => {
      // eslint-disable-next-line no-param-reassign
      map[obj.address] = obj.label;
      return map;
    }, {});

    return (
      <div>
        <div className={[cstyles.xlarge, cstyles.padall, cstyles.center].join(' ')}>Transactions</div>

        {/* Change the hardcoded height */}
        <ScrollPane offsetHeight={100}>
          {/* If no transactions, show the "loading..." text */
          !transactions && <div className={[cstyles.center, cstyles.margintoplarge].join(' ')}>Loading...</div>}

          {transactions && transactions.length === 0 && (
            <div className={[cstyles.center, cstyles.margintoplarge].join(' ')}>No Transactions Yet</div>
          )}
          {transactions &&
            transactions.map(t => {
              const key = t.type + t.txid + t.address;
              return (
                <TxItemBlock
                  key={key}
                  transaction={t}
                  currencyName={info.currencyName}
                  zecPrice={info.zecPrice}
                  txClicked={this.txClicked}
                  addressBookMap={addressBookMap}
                />
              );
            })}
        </ScrollPane>

        <TxModal
          modalIsOpen={modalIsOpen}
          tx={clickedTx}
          closeModal={this.closeModal}
          currencyName={info.currencyName}
          zecPrice={info.zecPrice}
        />
      </div>
    );
  }
}
