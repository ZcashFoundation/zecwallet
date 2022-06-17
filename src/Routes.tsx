/* eslint-disable react/jsx-props-no-spreading */
/* eslint-disable max-classes-per-file */
/* eslint-disable react/prop-types */
/* eslint-disable react/no-unused-state */
import React, { ReactElement } from "react";
import ReactModal from "react-modal";
import { Route, Routes } from "react-router-dom";
import { ErrorModal, ErrorModalData } from "./components/ErrorModal";
import cstyles from "./components/Common.module.css";
import routes from "./constants/routes.json";
import Dashboard from "./components/Dashboard";
import Send from "./components/Send";
import Receive from "./components/Receive";
import LoadingScreen from "./components/LoadingScreen";
import AppState, {
  AddressBalance,
  TotalBalance,
  Transaction,
  SendPageState,
  ToAddr,
  RPCConfig,
  Info,
  ReceivePageState,
  AddressBookEntry,
  AddressType,
} from "./components/AppState";
import RPC from "./rpc";
import Utils from "./utils/utils";
import { ZcashURITarget } from "./utils/uris";
import Zcashd from "./components/Zcashd";
import AddressBook from "./components/Addressbook";
import AddressbookImpl from "./utils/AddressbookImpl";
import Sidebar from "./components/Sidebar";
import Transactions from "./components/Transactions";

type Props = {};

export default class RouteApp extends React.Component<Props, AppState> {
  rpc: RPC | undefined;

  constructor(props: Props) {
    super(props);

    this.state = new AppState();

    // Create the initial ToAddr box
    // eslint-disable-next-line react/destructuring-assignment
    this.state.sendPageState.toaddrs = [new ToAddr(Utils.getNextToAddrID())];

    // Set the Modal's app element
    ReactModal.setAppElement("#root");
  }

  componentDidMount() {
    if (!this.rpc) {
      this.rpc = new RPC(
        this.setInfo,
        this.setTotalBalance,
        this.setAddressesWithBalances,
        this.setTransactionList,
        this.setAllAddresses,
        this.setZecPrice,
        this.setDisconnected
      );
    }

    // Read the address book
    (async () => {
      const addressBook = await AddressbookImpl.readAddressBook();
      if (addressBook) {
        this.setState({ addressBook });
      }
    })();
  }

  componentWillUnmount() {}

  getFullState = (): AppState => {
    return this.state;
  };

  openErrorModal = (title: string, body: string | ReactElement) => {
    const errorModalData = new ErrorModalData();
    errorModalData.modalIsOpen = true;
    errorModalData.title = title;
    errorModalData.body = body;

    this.setState({ errorModalData });
  };

  closeErrorModal = () => {
    const errorModalData = new ErrorModalData();
    errorModalData.modalIsOpen = false;

    this.setState({ errorModalData });
  };

  // Set the state of the current info object to be disconnected
  setDisconnected = (err: string) => {
    const { info } = this.state;

    const newInfo = new Info();
    Object.assign(newInfo, info);
    newInfo.disconnected = true;

    this.setState({ info: newInfo });
    this.openErrorModal("Disconnected", err);
  };

  setTotalBalance = (totalBalance: TotalBalance) => {
    this.setState({ totalBalance });
  };

  setAddressesWithBalances = (addressesWithBalance: AddressBalance[]) => {
    this.setState({ addressesWithBalance });

    const { sendPageState } = this.state;

    // If there is no 'from' address, we'll set a default one
    if (!sendPageState.fromaddr) {
      // Find a z-address with the highest balance
      const defaultAB = addressesWithBalance
        .filter((ab) => Utils.isSapling(ab.address))
        .reduce((prev: AddressBalance | null, ab) => {
          // We'll start with a sapling address
          if (prev == null) {
            return ab;
          }
          // Find the sapling address with the highest balance
          if (prev.balance < ab.balance) {
            return ab;
          }

          return prev;
        }, null);

      if (defaultAB) {
        const newSendPageState = sendPageState.clone();
        newSendPageState.fromaddr = defaultAB.address;

        this.setState({ sendPageState: newSendPageState });
      }
    }
  };

  setTransactionList = (transactions: Transaction[]) => {
    this.setState({ transactions });
  };

  setAllAddresses = (addresses: string[]) => {
    this.setState({ addresses });
  };

  setSendPageState = (sendPageState: SendPageState) => {
    this.setState({ sendPageState });
  };

  importPrivKeys = async (keys: string[]) => {
    if (!this.rpc) {
      return;
    }

    // eslint-disable-next-line no-plusplus
    for (let i = 0; i < keys.length; i++) {
      // The last doImport will take forever, because it will trigger the rescan. So, show
      // the dialog. If the last one fails, there will be an error displayed anyways
      if (i === keys.length - 1) {
        this.openErrorModal(
          "Key Import Started",
          <span>
            The import process for the private keys has started.
            <br />
            This will take a long time, up to 6 hours!
            <br />
            Please be patient!
          </span>
        );
      }

      const result = await this.rpc.doImportPrivKey(
        keys[i],
        i === keys.length - 1
      );
      if (result !== "") {
        this.openErrorModal(
          "Failed to import key",
          <span>
            <>
              A private key failed to import.
              <br />
              The error was:
              <br />
              {result}
            </>
          </span>
        );

        return;
      }
    }
  };

  setSendTo = (targets: ZcashURITarget[] | ZcashURITarget) => {
    // Clear the existing send page state and set up the new one
    const { sendPageState } = this.state;

    const newSendPageState = sendPageState.clone();
    newSendPageState.toaddrs = [];

    // If a single object is passed, accept that as well.
    let tgts = targets;
    if (!Array.isArray(tgts)) {
      tgts = [targets as ZcashURITarget];
    }

    tgts.forEach((tgt) => {
      const to = new ToAddr(Utils.getNextToAddrID());
      if (tgt.address) {
        to.to = tgt.address;
      }
      if (tgt.amount) {
        to.amount = tgt.amount;
      }
      if (tgt.memoString) {
        to.memo = tgt.memoString;
      }

      newSendPageState.toaddrs.push(to);
    });

    this.setState({ sendPageState: newSendPageState });
  };

  setRPCConfig = (rpcConfig?: RPCConfig) => {
    if (rpcConfig) {
      this.setState({ rpcConfig });
      console.log(rpcConfig);
      if (this.rpc) {
        this.rpc.configure(rpcConfig);
      }
    }
  };

  setZecPrice = (price: number | null) => {
    console.log(`Price = ${price}`);
    const { info } = this.state;

    const newInfo = new Info();
    Object.assign(newInfo, info);
    newInfo.zecPrice = price;

    this.setState({ info: newInfo });
  };

  setInfo = (newInfo: Info) => {
    // If the price is not set in this object, copy it over from the current object
    const { info } = this.state;
    if (!newInfo.zecPrice) {
      // eslint-disable-next-line no-param-reassign
      newInfo.zecPrice = info.zecPrice;
    }

    this.setState({ info: newInfo });
  };

  sendTransaction = async (
    sendJson: any[],
    fnOpenSendErrorModal: (title: string, msg: string) => void
  ): Promise<boolean> => {
    if (!this.rpc) {
      return false;
    }

    try {
      const success = await this.rpc.sendTransaction(
        sendJson,
        fnOpenSendErrorModal
      );
      return success;
    } catch (err) {
      console.log("route sendtx error", err);
      return false;
    }
  };

  // Get a single private key for this address, and return it as a string.
  getPrivKeyAsString = async (address: string): Promise<string> => {
    if (!this.rpc) {
      return "";
    }

    return this.rpc.getPrivKeyAsString(address);
  };

  // Getter methods, which are called by the components to update the state
  fetchAndSetSinglePrivKey = async (address: string) => {
    if (this.rpc) {
      const key = await this.rpc.getPrivKeyAsString(address);
      const addressPrivateKeys = new Map<string, string>();
      addressPrivateKeys.set(address, key);

      this.setState({ addressPrivateKeys });
    }
  };

  fetchAndSetSingleViewKey = async (address: string) => {
    if (!this.rpc) {
      return;
    }

    const key = await this.rpc.getViewKeyAsString(address);
    const addressViewKeys = new Map<string, string>();
    addressViewKeys.set(address, key);

    this.setState({ addressViewKeys });
  };

  addAddressBookEntry = (label: string, address: string) => {
    // Add an entry into the address book
    const { addressBook } = this.state;
    const newAddressBook = addressBook.concat(
      new AddressBookEntry(label, address)
    );

    // Write to disk. This method is async
    AddressbookImpl.writeAddressBook(newAddressBook);

    this.setState({ addressBook: newAddressBook });
  };

  removeAddressBookEntry = (label: string) => {
    const { addressBook } = this.state;
    const newAddressBook = addressBook.filter((i) => i.label !== label);

    // Write to disk. This method is async
    AddressbookImpl.writeAddressBook(newAddressBook);

    this.setState({ addressBook: newAddressBook });
  };

  createNewAddress = async (type: AddressType) => {
    if (!this.rpc) {
      return;
    }

    // Create a new address
    const newaddress = await this.rpc.createNewAddress(type);
    console.log(`Created new Address ${newaddress}`);

    // And then fetch the list of addresses again to refresh
    this.rpc.fetchAllAddresses();

    const { receivePageState } = this.state;
    const newRerenderKey = receivePageState.rerenderKey + 1;

    const newReceivePageState = new ReceivePageState();
    newReceivePageState.newAddress = newaddress;
    newReceivePageState.rerenderKey = newRerenderKey;

    this.setState({ receivePageState: newReceivePageState });
  };

  doRefresh = () => {
    if (this.rpc) {
      this.rpc.refresh(null);
    }
  };

  render() {
    const {
      totalBalance,
      transactions,
      addressesWithBalance,
      addressPrivateKeys,
      addressViewKeys,
      addresses,
      addressBook,
      sendPageState,
      receivePageState,
      info,
      errorModalData,
    } = this.state;

    const standardProps = {
      openErrorModal: this.openErrorModal,
      closeErrorModal: this.closeErrorModal,
      setSendTo: this.setSendTo,
      info,
    };

    return (
      <>
        <ErrorModal
          title={errorModalData.title}
          body={errorModalData.body}
          modalIsOpen={errorModalData.modalIsOpen}
          closeModal={this.closeErrorModal}
        />

        <div style={{ overflow: "hidden" }}>
          {info && info.version && (
            <div className={cstyles.sidebarcontainer}>
              <Sidebar
                getPrivKeyAsString={this.getPrivKeyAsString}
                importPrivKeys={this.importPrivKeys}
                addresses={addresses}
                transactions={transactions}
                {...standardProps}
              />
            </div>
          )}
          <div className={cstyles.contentcontainer}>
            <Routes>
              <Route
                path={routes.SEND}
                element={
                  <Send
                    addressesWithBalance={addressesWithBalance}
                    sendTransaction={this.sendTransaction}
                    sendPageState={sendPageState}
                    setSendPageState={this.setSendPageState}
                    addressBook={addressBook}
                    {...standardProps}
                  />
                }
              />
              <Route
                path={routes.RECEIVE}
                element={
                  <Receive
                    rerenderKey={receivePageState.rerenderKey}
                    addresses={addresses}
                    addressesWithBalance={addressesWithBalance}
                    addressPrivateKeys={addressPrivateKeys}
                    addressViewKeys={addressViewKeys}
                    receivePageState={receivePageState}
                    addressBook={addressBook}
                    {...standardProps}
                    fetchAndSetSinglePrivKey={this.fetchAndSetSinglePrivKey}
                    fetchAndSetSingleViewKey={this.fetchAndSetSingleViewKey}
                    createNewAddress={this.createNewAddress}
                  />
                }
              />
              <Route
                path={routes.ADDRESSBOOK}
                element={
                  <AddressBook
                    addressBook={addressBook}
                    addAddressBookEntry={this.addAddressBookEntry}
                    removeAddressBookEntry={this.removeAddressBookEntry}
                    {...standardProps}
                  />
                }
              />
              <Route
                path={routes.DASHBOARD}
                // eslint-disable-next-line react/jsx-props-no-spreading
                element={
                  <Dashboard
                    totalBalance={totalBalance}
                    info={info}
                    addressesWithBalance={addressesWithBalance}
                  />
                }
              />
              <Route
                path={routes.TRANSACTIONS}
                element={
                  <Transactions
                    transactions={transactions}
                    info={info}
                    addressBook={addressBook}
                    setSendTo={this.setSendTo}
                  />
                }
              />

              <Route
                path={routes.ZCASHD}
                element={<Zcashd info={info} refresh={this.doRefresh} />}
              />

              <Route
                path={routes.LOADING}
                element={
                  <LoadingScreen
                    setRPCConfig={this.setRPCConfig}
                    setInfo={this.setInfo}
                  />
                }
              />
            </Routes>
          </div>
        </div>
      </>
    );
  }
}
