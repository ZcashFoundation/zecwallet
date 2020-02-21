/* eslint-disable react/prop-types */
import React, { Component, useState } from 'react';
import { Tab, Tabs, TabList, TabPanel } from 'react-tabs';
import {
  Accordion,
  AccordionItem,
  AccordionItemHeading,
  AccordionItemButton,
  AccordionItemPanel
} from 'react-accessible-accordion';
import QRCode from 'qrcode.react';
import { clipboard } from 'electron';
import styles from './Receive.css';
import cstyles from './Common.css';
import Utils from '../utils/utils';
import { AddressBalance, Info, ReceivePageState, AddressBookEntry } from './AppState';
import ScrollPane from './ScrollPane';

const AddressBlock = ({ addressBalance, label, currencyName, zecPrice, privateKey, fetchAndSetSinglePrivKey }) => {
  const { address } = addressBalance;

  const [copied, setCopied] = useState(false);
  const balance = addressBalance.balance || 0;

  return (
    <AccordionItem key={copied} className={[cstyles.well, styles.receiveblock].join(' ')} uuid={address}>
      <AccordionItemHeading>
        <AccordionItemButton className={cstyles.accordionHeader}>{address}</AccordionItemButton>
      </AccordionItemHeading>
      <AccordionItemPanel className={[styles.receiveDetail].join(' ')}>
        <div className={[cstyles.flexspacebetween].join(' ')}>
          <div className={[cstyles.verticalflex, cstyles.marginleft].join(' ')}>
            {/*
            <div className={[cstyles.sublight].join(' ')}>Address</div>
            <div className={[cstyles.padtopsmall, cstyles.fixedfont].join(' ')}>
              {Utils.splitStringIntoChunks(address, 6).join(' ')}
            </div>
            */}

            {label && (
              <div className={cstyles.margintoplarge}>
                <div className={[cstyles.sublight].join(' ')}>Label</div>
                <div className={[cstyles.padtopsmall, cstyles.fixedfont].join(' ')}>{label}</div>
              </div>
            )}

            <div className={[cstyles.sublight, cstyles.margintoplarge].join(' ')}>Funds</div>
            <div className={[cstyles.padtopsmall].join(' ')}>
              {currencyName} {balance}
            </div>
            <div className={[cstyles.padtopsmall].join(' ')}>{Utils.getZecToUsdString(zecPrice, balance)}</div>

            <div className={[cstyles.margintoplarge, cstyles.breakword].join(' ')}>
              {privateKey && (
                <div>
                  <div className={[cstyles.sublight].join(' ')}>Private Key</div>
                  <div
                    className={[cstyles.breakword, cstyles.padtopsmall, cstyles.fixedfont].join(' ')}
                    style={{ maxWidth: '600px' }}
                  >
                    {privateKey}
                  </div>
                </div>
              )}
            </div>

            <div>
              <button
                className={[cstyles.primarybutton, cstyles.margintoplarge].join(' ')}
                type="button"
                onClick={() => {
                  clipboard.writeText(address);
                  setCopied(true);
                  setTimeout(() => setCopied(false), 5000);
                }}
              >
                {copied ? <span>Copied!</span> : <span>Copy Address</span>}
              </button>
              {!privateKey && (
                <button
                  className={[cstyles.primarybutton].join(' ')}
                  type="button"
                  onClick={() => fetchAndSetSinglePrivKey(address)}
                >
                  Export Private Key
                </button>
              )}
            </div>
          </div>
          <div>
            <QRCode value={address} className={[styles.receiveQrcode].join(' ')} />
          </div>
        </div>
      </AccordionItemPanel>
    </AccordionItem>
  );
};

type Props = {
  addresses: string[],
  addressesWithBalance: AddressBalance[],
  addressBook: AddressBookEntry[],
  info: Info,
  receivePageState: ReceivePageState,
  fetchAndSetSinglePrivKey: string => void,
  createNewAddress: boolean => void,
  rerenderKey: number
};

export default class Receive extends Component<Props> {
  render() {
    const {
      addresses,
      addressesWithBalance,
      addressPrivateKeys,
      addressBook,
      info,
      receivePageState,
      fetchAndSetSinglePrivKey,
      createNewAddress,
      rerenderKey
    } = this.props;

    // Convert the addressBalances into a map.
    const addressMap = addressesWithBalance.reduce((map, a) => {
      // eslint-disable-next-line no-param-reassign
      map[a.address] = a.balance;
      return map;
    }, {});

    const zaddrs = addresses
      .filter(a => Utils.isSapling(a))
      .slice(0, 100)
      .map(a => new AddressBalance(a, addressMap[a]));

    let defaultZaddr = zaddrs.length ? zaddrs[0].address : '';
    if (receivePageState && Utils.isSapling(receivePageState.newAddress)) {
      defaultZaddr = receivePageState.newAddress;

      // move this address to the front, since the scrollbar will reset when we re-render
      zaddrs.sort((x, y) => {
        // eslint-disable-next-line prettier/prettier, no-nested-ternary
        return x.address === defaultZaddr ? -1 : y.address === defaultZaddr ? 1 : 0
      });
    }

    const taddrs = addresses
      .filter(a => Utils.isTransparent(a))
      .slice(0, 100)
      .map(a => new AddressBalance(a, addressMap[a]));

    let defaultTaddr = taddrs.length ? taddrs[0].address : '';
    if (receivePageState && Utils.isTransparent(receivePageState.newAddress)) {
      defaultTaddr = receivePageState.newAddress;

      // move this address to the front, since the scrollbar will reset when we re-render
      taddrs.sort((x, y) => {
        // eslint-disable-next-line prettier/prettier, no-nested-ternary
        return x.address === defaultTaddr ? -1 : y.address === defaultTaddr ? 1 : 0
      });
    }

    const addressBookMap = addressBook.reduce((map, obj) => {
      // eslint-disable-next-line no-param-reassign
      map[obj.address] = obj.label;
      return map;
    }, {});

    return (
      <div>
        <div className={styles.receivecontainer}>
          <Tabs>
            <TabList>
              <Tab>Shielded</Tab>
              <Tab>Transparent</Tab>
            </TabList>

            <TabPanel key={`z${rerenderKey}`}>
              {/* Change the hardcoded height */}
              <ScrollPane offsetHeight={100}>
                <Accordion preExpanded={[defaultZaddr]}>
                  {zaddrs.map(a => (
                    <AddressBlock
                      key={a.address}
                      addressBalance={a}
                      currencyName={info.currencyName}
                      label={addressBookMap[a.address]}
                      zecPrice={info.zecPrice}
                      privateKey={addressPrivateKeys[a.address]}
                      fetchAndSetSinglePrivKey={fetchAndSetSinglePrivKey}
                      rerender={this.rerender}
                    />
                  ))}
                </Accordion>

                <button
                  className={[cstyles.primarybutton, cstyles.margintoplarge, cstyles.marginbottomlarge].join(' ')}
                  onClick={() => createNewAddress(true)}
                  type="button"
                >
                  New Shielded Address
                </button>
              </ScrollPane>
            </TabPanel>

            <TabPanel key={`t${rerenderKey}`}>
              {/* Change the hardcoded height */}
              <ScrollPane offsetHeight={100}>
                <Accordion preExpanded={[defaultTaddr]}>
                  {taddrs.map(a => (
                    <AddressBlock
                      key={a.address}
                      addressBalance={a}
                      currencyName={info.currencyName}
                      zecPrice={info.zecPrice}
                      privateKey={addressPrivateKeys[a.address]}
                      fetchAndSetSinglePrivKey={fetchAndSetSinglePrivKey}
                      rerender={this.rerender}
                    />
                  ))}
                </Accordion>

                <button
                  className={[cstyles.primarybutton, cstyles.margintoplarge, cstyles.marginbottomlarge].join(' ')}
                  type="button"
                  onClick={() => createNewAddress(false)}
                >
                  New Transparent Address
                </button>
              </ScrollPane>
            </TabPanel>
          </Tabs>
        </div>
      </div>
    );
  }
}
