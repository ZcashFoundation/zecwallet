import React, { Component } from "react";
import {
  AccordionItemButton,
  AccordionItem,
  AccordionItemHeading,
  AccordionItemPanel,
  Accordion,
} from "react-accessible-accordion";
import styles from "./Dashboard.module.css";
import cstyles from "./Common.module.css";
import { TotalBalance, Info, AddressBalance } from "./AppState";
import Utils from "../utils/utils";
import ScrollPane from "./ScrollPane";

type BalanceBlockType = {
  zecValue: number;
  usdValue: string;
  currencyName: string;
  topLabel?: string;
};
export const BalanceBlockHighlight = ({
  zecValue,
  usdValue,
  currencyName,
}: BalanceBlockType) => {
  const { bigPart, smallPart } = Utils.splitZecAmountIntoBigSmall(zecValue);

  return (
    <div style={{ padding: "1em" }}>
      <div className={[cstyles.highlight, cstyles.xlarge].join(" ")}>
        <span>
          {currencyName} {bigPart}
        </span>
        <span className={[cstyles.small, cstyles.zecsmallpart].join(" ")}>
          {smallPart}
        </span>
      </div>
      <div className={[cstyles.sublight, cstyles.small].join(" ")}>
        {usdValue}
      </div>
    </div>
  );
};

const BalanceBlock = ({
  zecValue,
  usdValue,
  topLabel,
  currencyName,
}: BalanceBlockType) => {
  const { bigPart, smallPart } = Utils.splitZecAmountIntoBigSmall(zecValue);

  return (
    <div className={cstyles.padall}>
      <div className={[styles.sublight, styles.small].join(" ")}>
        {topLabel}
      </div>
      <div className={[cstyles.highlight, cstyles.large].join(" ")}>
        <span>
          {currencyName} {bigPart}
        </span>
        <span className={[cstyles.small, cstyles.zecsmallpart].join(" ")}>
          {smallPart}
        </span>
      </div>
      <div className={[cstyles.sublight, cstyles.small].join(" ")}>
        {usdValue}
      </div>
    </div>
  );
};

type AddressBalanceItemProps = {
  currencyName: string;
  zecPrice: number | null;
  item: AddressBalance;
};
const AddressBalanceItem = ({
  currencyName,
  zecPrice,
  item,
}: AddressBalanceItemProps) => {
  const { bigPart, smallPart } = Utils.splitZecAmountIntoBigSmall(
    Math.abs(item.balance)
  );

  return (
    <AccordionItem
      key={item.address}
      className={[cstyles.well, cstyles.margintopsmall].join(" ")}
      uuid={item.address.replaceAll(" ", "_")}
    >
      <AccordionItemHeading>
        <AccordionItemButton className={cstyles.accordionHeader}>
          <div className={[cstyles.flexspacebetween].join(" ")}>
            <div>{Utils.splitStringIntoChunks(item.address, 6).join(" ")}</div>
            <div className={[styles.txamount, cstyles.right].join(" ")}>
              <div>
                <span>
                  {currencyName} {bigPart}
                </span>
                <span
                  className={[cstyles.small, cstyles.zecsmallpart].join(" ")}
                >
                  {smallPart}
                </span>
              </div>
              <div
                className={[
                  cstyles.sublight,
                  cstyles.small,
                  cstyles.padtopsmall,
                ].join(" ")}
              >
                {Utils.getZecToUsdString(zecPrice, Math.abs(item.balance))}
              </div>
            </div>
          </div>
        </AccordionItemButton>
      </AccordionItemHeading>
      <AccordionItemPanel />
    </AccordionItem>
  );
};

type Props = {
  totalBalance: TotalBalance;
  info: Info;
  addressesWithBalance: AddressBalance[];
};

export default class Home extends Component<Props> {
  render() {
    const { totalBalance, info, addressesWithBalance } = this.props;

    return (
      <div>
        <div className={[cstyles.well, styles.balancebox].join(" ")}>
          <BalanceBlockHighlight
            zecValue={totalBalance.total}
            usdValue={Utils.getZecToUsdString(
              info.zecPrice,
              totalBalance.total
            )}
            currencyName={info.currencyName}
          />
          <BalanceBlock
            topLabel="Shielded"
            zecValue={totalBalance.private}
            usdValue={Utils.getZecToUsdString(
              info.zecPrice,
              totalBalance.private
            )}
            currencyName={info.currencyName}
          />
          <BalanceBlock
            topLabel="Transparent"
            zecValue={totalBalance.transparent}
            usdValue={Utils.getZecToUsdString(
              info.zecPrice,
              totalBalance.transparent
            )}
            currencyName={info.currencyName}
          />
        </div>

        <div className={styles.addressbalancecontainer}>
          <ScrollPane offsetHeight={200}>
            <div className={styles.addressbooklist}>
              <div
                className={[
                  cstyles.flexspacebetween,
                  cstyles.tableheader,
                  cstyles.sublight,
                ].join(" ")}
              >
                <div>Address</div>
                <div>Balance</div>
              </div>
              {addressesWithBalance &&
                (addressesWithBalance.length === 0 ? (
                  <div className={[cstyles.center, cstyles.sublight].join(" ")}>
                    No Addresses with a balance
                  </div>
                ) : (
                  <Accordion>
                    {addressesWithBalance
                      .filter((ab) => ab.balance > 0)
                      .map((ab) => (
                        <AddressBalanceItem
                          key={ab.address}
                          item={ab}
                          currencyName={info.currencyName}
                          zecPrice={info.zecPrice}
                        />
                      ))}
                  </Accordion>
                ))}
            </div>
          </ScrollPane>
        </div>
      </div>
    );
  }
}
