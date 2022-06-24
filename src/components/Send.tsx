import React, { ChangeEvent, PureComponent } from "react";
import Modal from "react-modal";
import Select from "react-select";
import TextareaAutosize from "react-textarea-autosize";
import styles from "./Send.module.css";
import cstyles from "./Common.module.css";
import {
  ToAddr,
  AddressBalance,
  SendPageState,
  Info,
  AddressBookEntry,
} from "./AppState";
import Utils from "../utils/utils";
import ScrollPane from "./ScrollPane";
import ArrowUpLight from "../assets/img/arrow_up_dark.png";
import { ErrorModal } from "./ErrorModal";
import routes from "../constants/routes.json";
import { useNavigate } from "react-router-dom";
import { isNull } from "underscore";

const Spacer = () => {
  return <div style={{ marginTop: "24px" }} />;
};

class ValueLabel {
  value: string;
  label: string;

  constructor(value?: string, label?: string) {
    this.value = value || "";
    this.label = label || "";
  }
}

type ToAddrBoxProps = {
  toaddr: ToAddr;
  zecPrice: number | null;
  updateToField: (
    id: number,
    address: ChangeEvent<HTMLInputElement> | null,
    amount: ChangeEvent<HTMLInputElement> | null,
    memo: ChangeEvent<HTMLTextAreaElement> | string | null
  ) => void;
  fromAddress: string;
  fromAmount: number;
  setMaxAmount: (id: number, total: number) => void;
  setSendButtonEnable: (enabled: boolean) => void;
  totalAmountAvailable: number;
};
const ToAddrBox = ({
  toaddr,
  zecPrice,
  updateToField,
  fromAddress,
  fromAmount,
  setMaxAmount,
  setSendButtonEnable,
  totalAmountAvailable,
}: ToAddrBoxProps) => {
  const isMemoDisabled = Utils.isTransparent(toaddr.to);

  const addressIsValid =
    toaddr.to === "" ||
    Utils.isUnified(toaddr.to) ||
    Utils.isZaddr(toaddr.to) ||
    Utils.isTransparent(toaddr.to);
  const memoIsValid =
    isNull(toaddr.memo) || (toaddr.memo && toaddr.memo.length <= 512);

  let amountError = null;
  if (toaddr.amount) {
    if (toaddr.amount < 0) {
      amountError = "Amount cannot be negative";
    }
    // if (toaddr.amount > fromAmount) {
    //   amountError = "Amount Exceeds Balance";
    // }
    if (toaddr.amount < 10 ** -8) {
      amountError = "Amount is too small";
    }
    const s = toaddr.amount.toString().split(".");
    if (s && s.length > 1 && s[1].length > 8) {
      amountError = "Too Many Decimals";
    }
  }

  if (isNaN(toaddr.amount)) {
    // Amount is empty
    amountError = "Amount cannot be empty";
  }

  let buttonstate = false;
  if (
    !addressIsValid ||
    amountError ||
    !memoIsValid ||
    toaddr.to === "" ||
    toaddr.amount === 0 ||
    fromAmount === 0
  ) {
    buttonstate = false;
  } else {
    buttonstate = true;
  }

  setTimeout(() => {
    console.log(`Setting send button state to ${buttonstate}`);
    setSendButtonEnable(buttonstate);
  }, 10);

  const usdValue = Utils.getZecToUsdString(zecPrice, toaddr.amount);

  const addReplyTo = () => {
    if (toaddr.memo && toaddr.memo.endsWith(fromAddress)) {
      return;
    }

    if (fromAddress) {
      updateToField(
        toaddr.id,
        null,
        null,
        `${toaddr.memo}\nReply-To:\n${fromAddress}`
      );
    }
  };

  return (
    <div>
      <div className={[cstyles.well, cstyles.verticalflex].join(" ")}>
        <div className={[cstyles.flexspacebetween].join(" ")}>
          <div className={cstyles.sublight}>To</div>
          <div className={cstyles.validationerror}>
            {addressIsValid ? (
              <i className={[cstyles.green, "fas", "fa-check"].join(" ")} />
            ) : (
              <span className={cstyles.red}>Invalid Address</span>
            )}
          </div>
        </div>
        <input
          type="text"
          placeholder="U | Z | T address"
          className={cstyles.inputbox}
          value={toaddr.to}
          onChange={(e) => updateToField(toaddr.id, e, null, null)}
        />
        <Spacer />
        <div className={[cstyles.flexspacebetween].join(" ")}>
          <div className={cstyles.sublight}>Amount</div>
          <div className={cstyles.validationerror}>
            {amountError ? (
              <span className={cstyles.red}>{amountError}</span>
            ) : (
              <span>{usdValue}</span>
            )}
          </div>
        </div>
        <div className={[cstyles.flexspacebetween].join(" ")}>
          <input
            type="number"
            step="any"
            className={cstyles.inputbox}
            value={isNaN(toaddr.amount) ? "" : toaddr.amount}
            onChange={(e) => updateToField(toaddr.id, null, e, null)}
          />
          <img
            className={styles.toaddrbutton}
            src={ArrowUpLight}
            alt="Max"
            onClick={() => setMaxAmount(toaddr.id, totalAmountAvailable)}
          />
        </div>

        <Spacer />

        {isMemoDisabled && (
          <div className={cstyles.sublight}>Memos only for z-addresses</div>
        )}

        {!isMemoDisabled && (
          <div>
            <div className={[cstyles.flexspacebetween].join(" ")}>
              <div className={cstyles.sublight}>Memo</div>
              <div className={cstyles.validationerror}>
                {memoIsValid ? (
                  toaddr.memo && toaddr.memo.length
                ) : (
                  <span className={cstyles.red}>
                    {toaddr.memo && toaddr.memo.length}
                  </span>
                )}{" "}
                / 512
              </div>
            </div>
            <TextareaAutosize
              className={cstyles.inputbox}
              value={toaddr.memo || ""}
              disabled={isMemoDisabled}
              onChange={(e) => updateToField(toaddr.id, null, null, e)}
            />
            <input
              type="checkbox"
              onChange={(e) => e.target.checked && addReplyTo()}
            />
            Include Reply-To address
          </div>
        )}
        <Spacer />
      </div>
      <Spacer />
    </div>
  );
};

type ConfirmModalToAddrProps = { toaddr: ToAddr; info: Info };
const ConfirmModalToAddr = ({ toaddr, info }: ConfirmModalToAddrProps) => {
  const { bigPart, smallPart } = Utils.splitZecAmountIntoBigSmall(
    toaddr.amount
  );

  const memo: string = toaddr.memo ? toaddr.memo : "";

  return (
    <div className={cstyles.well}>
      <div
        className={[cstyles.flexspacebetween, cstyles.margintoplarge].join(" ")}
      >
        <div className={[styles.confirmModalAddress].join(" ")}>
          {Utils.splitStringIntoChunks(toaddr.to, 6).join(" ")}
        </div>
        <div className={[cstyles.verticalflex, cstyles.right].join(" ")}>
          <div className={cstyles.large}>
            <div>
              <span>
                {info.currencyName} {bigPart}
              </span>
              <span className={[cstyles.small, styles.zecsmallpart].join(" ")}>
                {smallPart}
              </span>
            </div>
          </div>
          <div>{Utils.getZecToUsdString(info.zecPrice, toaddr.amount)}</div>
        </div>
      </div>
      <div className={[cstyles.sublight, cstyles.breakword].join(" ")}>
        {memo}
      </div>
    </div>
  );
};

type ConfirmModalProps = {
  sendPageState: SendPageState;
  info: Info;
  getSendManyJSON: () => any[];
  sendTransaction: (
    sendJson: any[],
    fnOpenSendErrorModal: (title: string, msg: string) => void
  ) => Promise<boolean>;
  clearToAddrs: () => void;
  closeModal: () => void;
  modalIsOpen: boolean;
  openErrorModal: (title: string, body: string) => void;
};
const ConfirmModal = ({
  sendPageState,
  info,
  getSendManyJSON,
  sendTransaction,
  clearToAddrs,
  closeModal,
  modalIsOpen,
  openErrorModal,
}: ConfirmModalProps) => {
  const sendingTotal =
    sendPageState.toaddrs.reduce((s, t) => s + t.amount, 0.0) +
    Utils.getDefaultFee(info.latestBlock);
  const { bigPart, smallPart } = Utils.splitZecAmountIntoBigSmall(sendingTotal);

  const navigate = useNavigate();

  const sendButton = () => {
    // First, close the confirm modal.
    closeModal();

    // This will be replaced by either a success TXID or error message that the user
    // has to close manually.
    openErrorModal(
      "Computing Transaction",
      "Please wait...This could take a while"
    );

    // Then send the Tx async
    (async () => {
      const sendJson = getSendManyJSON();
      let success = false;

      try {
        success = await sendTransaction(sendJson, openErrorModal);
      } catch (err) {
        // If there was an error, show the error modal
        openErrorModal("Error Sending Transaction", err as string);
      }

      // If the Tx was sent, then clear the addresses
      if (success) {
        clearToAddrs();

        // Redirect to dashboard after
        navigate(routes.DASHBOARD);
      }
    })();
  };

  return (
    <Modal
      isOpen={modalIsOpen}
      onRequestClose={closeModal}
      className={styles.confirmModal}
      overlayClassName={styles.confirmOverlay}
    >
      <div className={[cstyles.verticalflex].join(" ")}>
        <div className={[cstyles.marginbottomlarge, cstyles.center].join(" ")}>
          Confirm Transaction
        </div>
        <div className={cstyles.flex}>
          <div
            className={[
              cstyles.highlight,
              cstyles.xlarge,
              cstyles.flexspacebetween,
              cstyles.well,
              cstyles.maxwidth,
            ].join(" ")}
          >
            <div>Total</div>
            <div className={[cstyles.right, cstyles.verticalflex].join(" ")}>
              <div>
                <span>
                  {info.currencyName} {bigPart}
                </span>
                <span
                  className={[cstyles.small, styles.zecsmallpart].join(" ")}
                >
                  {smallPart}
                </span>
              </div>

              <div className={cstyles.normal}>
                {Utils.getZecToUsdString(info.zecPrice, sendingTotal)}
              </div>
            </div>
          </div>
        </div>

        <div
          className={[cstyles.verticalflex, cstyles.margintoplarge].join(" ")}
        >
          {sendPageState.toaddrs.map((t) => (
            <ConfirmModalToAddr key={t.to} toaddr={t} info={info} />
          ))}
        </div>

        <ConfirmModalToAddr
          toaddr={{
            id: -1,
            to: "Fee",
            amount: Utils.getDefaultFee(info.latestBlock),
            memo: null,
          }}
          info={info}
        />

        {info && info.disconnected && (
          <div className={[cstyles.red, cstyles.margintoplarge].join(" ")}>
            You are currently disconnected. This transaction might not work.
          </div>
        )}

        {info && info.verificationProgress < 0.9999 && (
          <div className={[cstyles.red, cstyles.margintoplarge].join(" ")}>
            You are currently syncing. This transaction might not work.
          </div>
        )}

        <div className={cstyles.buttoncontainer}>
          <button
            type="button"
            className={cstyles.primarybutton}
            onClick={() => sendButton()}
          >
            Send
          </button>
          <button
            type="button"
            className={cstyles.primarybutton}
            onClick={closeModal}
          >
            Cancel
          </button>
        </div>
      </div>
    </Modal>
  );
};

type Props = {
  addressesWithBalance: AddressBalance[];
  addressBook: AddressBookEntry[];
  sendPageState: SendPageState;
  sendTransaction: (
    sendJson: any[],
    fnOpenSendErrorModal: (title: string, msg: string) => void
  ) => Promise<boolean>;
  setSendPageState: (sendPageState: SendPageState) => void;
  openErrorModal: (title: string, body: string) => void;
  closeErrorModal: () => void;
  info: Info;
  getSendManyJSON: () => any[];
};

class SendState {
  modalIsOpen: boolean;
  errorModalIsOpen: boolean;
  errorModalTitle: string;
  errorModalBody: string;
  sendButtonEnabled: boolean;

  constructor() {
    this.modalIsOpen = false;
    this.errorModalIsOpen = false;
    this.errorModalBody = "";
    this.errorModalTitle = "";
    this.sendButtonEnabled = false;
  }
}

export default class Send extends PureComponent<Props, SendState> {
  constructor(props: Props) {
    super(props);

    this.state = new SendState();
  }

  addToAddr = () => {
    const { sendPageState, setSendPageState } = this.props;
    const newToAddrs = sendPageState.toaddrs.concat(
      new ToAddr(Utils.getNextToAddrID())
    );

    // Create the new state object
    const newState = sendPageState.clone();
    newState.toaddrs = newToAddrs;

    setSendPageState(newState);
  };

  clearToAddrs = () => {
    const { sendPageState, setSendPageState } = this.props;
    const newToAddrs = [new ToAddr(Utils.getNextToAddrID())];

    // Create the new state object
    const newState = sendPageState.clone();
    newState.toaddrs = newToAddrs;

    setSendPageState(newState);
  };

  changeFrom = (event: any) => {
    const { sendPageState, setSendPageState } = this.props;

    // Create the new state object
    const newState = sendPageState.clone();
    newState.fromaddr = event.value;

    setSendPageState(newState);
  };

  changePrivacyLevel = (event: any) => {
    const { sendPageState, setSendPageState } = this.props;

    // Create the new state object
    const newState = sendPageState.clone();
    newState.privacyLevel = event.value;

    setSendPageState(newState);
  };

  updateToField = (
    id: number,
    address: ChangeEvent<HTMLInputElement> | null,
    amount: ChangeEvent<HTMLInputElement> | null,
    memo: ChangeEvent<HTMLTextAreaElement> | string | null
  ) => {
    const { sendPageState, setSendPageState } = this.props;

    const newToAddrs = sendPageState.toaddrs.slice(0);

    // Find the correct toAddr
    const toAddr = newToAddrs.find((a) => a.id === id) as ToAddr;
    if (address) {
      toAddr.to = address.target.value.replace(/ /g, ""); // Remove spaces
    }

    if (amount) {
      // Check to see the new amount if valid
      const newAmount = parseFloat(amount.target.value);
      if (newAmount < 0 || newAmount > 21 * 10 ** 6) {
        return;
      }
      toAddr.amount = newAmount;
    }

    if (memo) {
      if (typeof memo === "string") {
        toAddr.memo = memo;
      } else {
        toAddr.memo = memo.target.value;
      }
    }

    // Create the new state object
    const newState = sendPageState.clone();
    newState.toaddrs = newToAddrs;

    console.log(newState);

    setSendPageState(newState);
  };

  setMaxAmount = (id: number, total: number) => {
    const { sendPageState, setSendPageState, info } = this.props;

    const newToAddrs = sendPageState.toaddrs.slice(0);

    let totalOtherAmount: number = newToAddrs
      .filter((a) => a.id !== id)
      .reduce((s, a) => s + a.amount, 0);

    // Add Fee
    totalOtherAmount += Utils.getDefaultFee(info.latestBlock);

    // Find the correct toAddr
    const toAddr = newToAddrs.find((a) => a.id === id) as ToAddr;
    toAddr.amount = total - totalOtherAmount;
    if (toAddr.amount < 0) toAddr.amount = 0;
    toAddr.amount = parseFloat(Utils.maxPrecisionTrimmed(toAddr.amount));

    // Create the new state object
    const newState = sendPageState.clone();
    newState.toaddrs = newToAddrs;

    setSendPageState(newState);
  };

  setSendButtonEnable = (sendButtonEnabled: boolean) => {
    this.setState({ sendButtonEnabled });
  };

  openModal = () => {
    this.setState({ modalIsOpen: true });
  };

  closeModal = () => {
    this.setState({ modalIsOpen: false });
  };

  getBalanceForAddress = (
    addr: string,
    addressesWithBalance: AddressBalance[]
  ): number => {
    // Find the addr in addressesWithBalance
    const addressBalance = addressesWithBalance.find(
      (ab) => ab.address === addr
    );

    if (!addressBalance) {
      return 0;
    }

    return addressBalance.balance;
  };

  getLabelForFromAddress = (
    addr: string,
    addressesWithBalance: AddressBalance[],
    currencyName: string
  ) => {
    // Find the addr in addressesWithBalance
    const { addressBook } = this.props;
    const label = addressBook.find((ab) => ab.address === addr);
    const labelStr = label ? ` [ ${label.label} ]` : "";

    const balance = this.getBalanceForAddress(addr, addressesWithBalance);

    return `[ ${currencyName} ${balance.toString()} ]${labelStr} ${addr}`;
  };

  render() {
    const {
      modalIsOpen,
      errorModalIsOpen,
      errorModalTitle,
      errorModalBody,
      sendButtonEnabled,
    } = this.state;
    const {
      sendPageState,
      info,
      openErrorModal,
      closeErrorModal,
      getSendManyJSON,
    } = this.props;

    const customStyles = {
      option: (provided: any, state: any) => ({
        ...provided,
        color: state.isSelected ? "#c3921f;" : "white",
        background: "#212124;",
        padding: 20,
      }),
      menu: (provided: any) => ({
        ...provided,
        background: "#212124;",
      }),
      control: () => ({
        // none of react-select's styles are passed to <Control />
        display: "flex",
        alignItems: "center",
        // flexWrap: "flex",
        background: "#212124;",
      }),
      singleValue: (provided: any, state: any) => {
        const opacity = state.isDisabled ? 0.5 : 1;
        const transition = "opacity 300ms";

        return { ...provided, opacity, transition, color: "#ffffff" };
      },
    };

    const { addressesWithBalance, sendTransaction } = this.props;
    const sendFromList = addressesWithBalance
      .filter((ab) => !ab.address.includes("change"))
      .map((ab) => {
        return {
          value: ab.address,
          label: this.getLabelForFromAddress(
            ab.address,
            addressesWithBalance,
            info.currencyName
          ),
        };
      });

    // Find the fromaddress
    let fromaddr = new ValueLabel();
    if (sendPageState.fromaddr) {
      fromaddr = {
        value: sendPageState.fromaddr,
        label: this.getLabelForFromAddress(
          sendPageState.fromaddr,
          addressesWithBalance,
          info.currencyName
        ),
      };
    }

    const privacyLevelList = [
      "FullPrivacy",
      "AllowRevealedAmounts",
      "NoPrivacy",
    ].map((v) => new ValueLabel(v, v));
    let privacyLevel = privacyLevelList.find(
      (vl) => vl.value === sendPageState.privacyLevel
    );

    const totalAmountAvailable = this.getBalanceForAddress(
      fromaddr.value,
      addressesWithBalance
    );

    return (
      <div>
        <div
          className={[cstyles.xlarge, cstyles.padall, cstyles.center].join(" ")}
        >
          Send
        </div>

        <div className={styles.sendcontainer}>
          <div className={[cstyles.well, cstyles.verticalflex].join(" ")}>
            <div
              className={[cstyles.sublight, cstyles.padbottomsmall].join(" ")}
            >
              Send From
            </div>
            <Select
              value={fromaddr}
              options={sendFromList}
              styles={customStyles}
              onChange={this.changeFrom}
            />
          </div>

          <div className={[cstyles.well, cstyles.verticalflex].join(" ")}>
            <div
              className={[cstyles.sublight, cstyles.padbottomsmall].join(" ")}
            >
              Privacy Level
            </div>
            <Select
              value={privacyLevel}
              options={privacyLevelList}
              styles={customStyles}
              onChange={this.changePrivacyLevel}
            />
          </div>

          <Spacer />

          <ScrollPane offsetHeight={400}>
            {sendPageState.toaddrs.map((toaddr) => {
              return (
                <ToAddrBox
                  key={toaddr.id}
                  toaddr={toaddr}
                  zecPrice={info.zecPrice}
                  updateToField={this.updateToField}
                  fromAddress={fromaddr.value}
                  fromAmount={totalAmountAvailable}
                  setMaxAmount={this.setMaxAmount}
                  setSendButtonEnable={this.setSendButtonEnable}
                  totalAmountAvailable={totalAmountAvailable}
                />
              );
            })}
            <div style={{ textAlign: "right" }}>
              <button type="button" onClick={this.addToAddr}>
                <i className={["fas", "fa-plus"].join(" ")} />
              </button>
            </div>
          </ScrollPane>

          <div className={cstyles.center}>
            <button
              type="button"
              disabled={!sendButtonEnabled}
              className={cstyles.primarybutton}
              onClick={this.openModal}
            >
              Send
            </button>
            <button
              type="button"
              className={cstyles.primarybutton}
              onClick={this.clearToAddrs}
            >
              Cancel
            </button>
          </div>

          <ConfirmModal
            sendPageState={sendPageState}
            getSendManyJSON={getSendManyJSON}
            info={info}
            sendTransaction={sendTransaction}
            openErrorModal={openErrorModal}
            closeModal={this.closeModal}
            modalIsOpen={modalIsOpen}
            clearToAddrs={this.clearToAddrs}
          />

          <ErrorModal
            title={errorModalTitle}
            body={errorModalBody}
            modalIsOpen={errorModalIsOpen}
            closeModal={closeErrorModal}
          />
        </div>
      </div>
    );
  }
}
