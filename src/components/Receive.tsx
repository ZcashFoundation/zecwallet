import React, { Component, useState, useEffect } from "react";
import { Tab, Tabs, TabList, TabPanel } from "react-tabs";
import {
  Accordion,
  AccordionItem,
  AccordionItemHeading,
  AccordionItemButton,
  AccordionItemPanel,
} from "react-accessible-accordion";
import QRCode from "qrcode.react";
import styles from "./Receive.module.css";
import cstyles from "./Common.module.css";
import Utils from "../utils/utils";
import {
  AddressBalance,
  Info,
  ReceivePageState,
  AddressBookEntry,
  AddressType,
  AddressDetail,
} from "./AppState";
import ScrollPane from "./ScrollPane";

type AddressBlockProps = {
  addresses: AddressDetail[];
  addressBalance: AddressBalance;
  label?: string;
  currencyName: string;
  zecPrice: number | null;
  privateKey?: string;
  viewKey?: string;
  fetchAndSetSinglePrivKey?: (address: string) => void;
  fetchAndSetSingleViewKey?: (address: string) => void;
};

const AddressBlock = ({
  addresses,
  addressBalance,
  label,
  currencyName,
  zecPrice,
  privateKey,
  fetchAndSetSinglePrivKey,
  viewKey,
  fetchAndSetSingleViewKey,
}: AddressBlockProps) => {
  const { address } = addressBalance;

  const [copied, setCopied] = useState(false);
  const [timerID, setTimerID] = useState<NodeJS.Timeout | null>(null);

  useEffect(() => {
    return () => {
      if (timerID) {
        clearTimeout(timerID);
      }
    };
  });

  const balance = addressBalance.balance || 0;

  const openAddress = () => {
    if (currencyName === "TAZ") {
      window.zecwallet.openLink(`https://chain.so/address/ZECTEST/${address}`);
    } else {
      window.zecwallet.openLink(`https://zcha.in/accounts/${address}`);
    }
  };

  return (
    <AccordionItem
      key={`${copied}`}
      className={[cstyles.well, styles.receiveblock].join(" ")}
      uuid={address}
    >
      <AccordionItemHeading>
        <AccordionItemButton className={cstyles.accordionHeader}>
          {address}
        </AccordionItemButton>
      </AccordionItemHeading>
      <AccordionItemPanel className={[styles.receiveDetail].join(" ")}>
        <div className={[cstyles.flexspacebetween].join(" ")}>
          <div className={[cstyles.verticalflex, cstyles.marginleft].join(" ")}>
            {label && (
              <div className={cstyles.margintoplarge}>
                <div className={[cstyles.sublight].join(" ")}>Label</div>
                <div
                  className={[cstyles.padtopsmall, cstyles.fixedfont].join(" ")}
                >
                  {label}
                </div>
              </div>
            )}

            {Utils.isUnified(address) && (
              <>
                <div
                  className={[cstyles.sublight, cstyles.margintoplarge].join(
                    " "
                  )}
                >
                  Details
                </div>
                <div className={[cstyles.padtopsmall].join(" ")}>
                  Account Number:{" "}
                  {addresses.find((ad) => ad.address === address)?.account}
                </div>
                <div className={[cstyles.padtopsmall].join(" ")}>
                  Diversifier Index:{" "}
                  {addresses.find((ad) => ad.address === address)?.diversifier}
                </div>
              </>
            )}

            <div
              className={[cstyles.sublight, cstyles.margintoplarge].join(" ")}
            >
              Funds
            </div>
            <div className={[cstyles.padtopsmall].join(" ")}>
              {currencyName} {balance}
            </div>
            <div className={[cstyles.padtopsmall].join(" ")}>
              {Utils.getZecToUsdString(zecPrice, balance)}
            </div>

            <div
              className={[cstyles.margintoplarge, cstyles.breakword].join(" ")}
            >
              {privateKey && (
                <div>
                  <div className={[cstyles.sublight].join(" ")}>
                    Private Key
                  </div>
                  <div
                    className={[
                      cstyles.breakword,
                      cstyles.padtopsmall,
                      cstyles.fixedfont,
                      cstyles.flex,
                    ].join(" ")}
                    style={{ maxWidth: "600px" }}
                  >
                    <QRCode
                      value={privateKey}
                      className={[styles.receiveQrcode].join(" ")}
                    />
                    <div>{privateKey}</div>
                  </div>
                </div>
              )}
            </div>

            <div
              className={[cstyles.margintoplarge, cstyles.breakword].join(" ")}
            >
              {viewKey && (
                <div>
                  <div className={[cstyles.sublight].join(" ")}>
                    Viewing Key
                  </div>
                  <div
                    className={[
                      cstyles.breakword,
                      cstyles.padtopsmall,
                      cstyles.fixedfont,
                      cstyles.flex,
                    ].join(" ")}
                    style={{ maxWidth: "600px" }}
                  >
                    <QRCode
                      value={viewKey}
                      className={[styles.receiveQrcode].join(" ")}
                    />
                    <div>{viewKey}</div>
                  </div>
                </div>
              )}
            </div>

            <div>
              <button
                className={[cstyles.primarybutton, cstyles.margintoplarge].join(
                  " "
                )}
                type="button"
                onClick={() => {
                  window.zecwallet.copyToClipboard(address);
                  setCopied(true);
                  setTimerID(setTimeout(() => setCopied(false), 5000));
                }}
              >
                {copied ? <span>Copied!</span> : <span>Copy Address</span>}
              </button>
              {!privateKey && fetchAndSetSinglePrivKey && (
                <button
                  className={[cstyles.primarybutton].join(" ")}
                  type="button"
                  onClick={() => fetchAndSetSinglePrivKey(address)}
                >
                  Export Private Key
                </button>
              )}

              {Utils.isZaddr(address) && !viewKey && fetchAndSetSingleViewKey && (
                <button
                  className={[cstyles.primarybutton].join(" ")}
                  type="button"
                  onClick={() => fetchAndSetSingleViewKey(address)}
                >
                  Export Viewing Key
                </button>
              )}

              {Utils.isTransparent(address) && (
                <button
                  className={[cstyles.primarybutton].join(" ")}
                  type="button"
                  onClick={() => openAddress()}
                >
                  View on explorer{" "}
                  <i
                    className={["fas", "fa-external-link-square-alt"].join(" ")}
                  />
                </button>
              )}
            </div>
          </div>
          <div>
            <QRCode
              value={address}
              className={[styles.receiveQrcode].join(" ")}
            />
          </div>
        </div>
      </AccordionItemPanel>
    </AccordionItem>
  );
};

type Props = {
  addresses: AddressDetail[];
  addressesWithBalance: AddressBalance[];
  addressBook: AddressBookEntry[];
  info: Info;
  receivePageState: ReceivePageState;
  fetchAndSetSinglePrivKey: (address: string) => void;
  fetchAndSetSingleViewKey: (address: string) => void;
  createNewAddress: (type: AddressType) => void;
  rerenderKey: number;
  addressPrivateKeys: Map<string, string>;
  addressViewKeys: Map<string, string>;
};

export default class Receive extends Component<Props> {
  render() {
    const {
      addresses,
      addressesWithBalance,
      addressPrivateKeys,
      addressViewKeys,
      addressBook,
      info,
      receivePageState,
      fetchAndSetSinglePrivKey,
      fetchAndSetSingleViewKey,
      createNewAddress,
      rerenderKey,
    } = this.props;

    // Convert the addressBalances into a map.
    const addressMap = addressesWithBalance.reduce((map, a) => {
      // eslint-disable-next-line no-param-reassign
      map.set(a.address, a.balance);
      return map;
    }, new Map<string, number>());

    const uaddrs = addresses
      .filter((a) => a.type === AddressType.unified)
      .slice(0, 100)
      .map(
        (a) => new AddressBalance(a.address, addressMap.get(a.address) || 0)
      );
    let defaultUaddr = uaddrs.length ? uaddrs[0].address : "";
    if (receivePageState && Utils.isUnified(receivePageState.newAddress)) {
      defaultUaddr = receivePageState.newAddress;

      // move this address to the front, since the scrollbar will reset when we re-render
      uaddrs.sort((x, y) => {
        return x.address === defaultUaddr
          ? -1
          : y.address === defaultUaddr
          ? 1
          : 0;
      });
    }

    const zaddrs = addresses
      .filter((a) => a.type === AddressType.sapling)
      .slice(0, 100)
      .map(
        (a) => new AddressBalance(a.address, addressMap.get(a.address) || 0)
      );

    let defaultZaddr = zaddrs.length ? zaddrs[0].address : "";
    if (receivePageState && Utils.isSapling(receivePageState.newAddress)) {
      defaultZaddr = receivePageState.newAddress;

      // move this address to the front, since the scrollbar will reset when we re-render
      zaddrs.sort((x, y) => {
        return x.address === defaultZaddr
          ? -1
          : y.address === defaultZaddr
          ? 1
          : 0;
      });
    }

    const taddrs = addresses
      .filter((a) => a.type === AddressType.transparent)
      .slice(0, 100)
      .map(
        (a) => new AddressBalance(a.address, addressMap.get(a.address) || 0)
      );

    let defaultTaddr = taddrs.length ? taddrs[0].address : "";
    if (receivePageState && Utils.isTransparent(receivePageState.newAddress)) {
      defaultTaddr = receivePageState.newAddress;

      // move this address to the front, since the scrollbar will reset when we re-render
      taddrs.sort((x, y) => {
        return x.address === defaultTaddr
          ? -1
          : y.address === defaultTaddr
          ? 1
          : 0;
      });
    }

    const addressBookMap = addressBook.reduce((map, obj) => {
      map.set(obj.address, obj.label);
      return map;
    }, new Map<string, string>());

    return (
      <div>
        <div className={styles.receivecontainer}>
          <Tabs>
            <TabList>
              <Tab>Unified</Tab>
              <Tab>Shielded</Tab>
              <Tab>Transparent</Tab>
            </TabList>

            <TabPanel key={`ua${rerenderKey}`}>
              <ScrollPane offsetHeight={100}>
                <Accordion preExpanded={[defaultUaddr]}>
                  {uaddrs.map((a) => (
                    <AddressBlock
                      key={a.address}
                      addresses={addresses}
                      addressBalance={a}
                      currencyName={info.currencyName}
                      label={addressBookMap.get(a.address)}
                      zecPrice={info.zecPrice}
                      privateKey={addressPrivateKeys.get(a.address)}
                      viewKey={addressViewKeys.get(a.address)}
                    />
                  ))}
                </Accordion>

                <button
                  className={[
                    cstyles.primarybutton,
                    cstyles.margintoplarge,
                    cstyles.marginbottomlarge,
                  ].join(" ")}
                  onClick={() => createNewAddress(AddressType.unified)}
                  type="button"
                >
                  New Unified Address
                </button>
              </ScrollPane>
            </TabPanel>

            <TabPanel key={`z${rerenderKey}`}>
              {/* Change the hardcoded height */}
              <ScrollPane offsetHeight={100}>
                <Accordion preExpanded={[defaultZaddr]}>
                  {zaddrs.map((a) => (
                    <AddressBlock
                      key={a.address}
                      addresses={addresses}
                      addressBalance={a}
                      currencyName={info.currencyName}
                      label={addressBookMap.get(a.address)}
                      zecPrice={info.zecPrice}
                      privateKey={addressPrivateKeys.get(a.address)}
                      viewKey={addressViewKeys.get(a.address)}
                      fetchAndSetSinglePrivKey={fetchAndSetSinglePrivKey}
                      fetchAndSetSingleViewKey={fetchAndSetSingleViewKey}
                    />
                  ))}
                </Accordion>

                <button
                  className={[
                    cstyles.primarybutton,
                    cstyles.margintoplarge,
                    cstyles.marginbottomlarge,
                  ].join(" ")}
                  onClick={() => createNewAddress(AddressType.sapling)}
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
                  {taddrs.map((a) => (
                    <AddressBlock
                      key={a.address}
                      addresses={addresses}
                      addressBalance={a}
                      currencyName={info.currencyName}
                      zecPrice={info.zecPrice}
                      privateKey={addressPrivateKeys.get(a.address)}
                      viewKey={addressViewKeys.get(a.address)}
                      fetchAndSetSinglePrivKey={fetchAndSetSinglePrivKey}
                    />
                  ))}
                </Accordion>

                <button
                  className={[
                    cstyles.primarybutton,
                    cstyles.margintoplarge,
                    cstyles.marginbottomlarge,
                  ].join(" ")}
                  type="button"
                  onClick={() => createNewAddress(AddressType.transparent)}
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
