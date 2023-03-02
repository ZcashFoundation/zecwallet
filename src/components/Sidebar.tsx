import React, { PureComponent, ReactElement } from "react";
import dateformat from "dateformat";
import Modal from "react-modal";
import TextareaAutosize from "react-textarea-autosize";
import styles from "./Sidebar.module.css";
import cstyles from "./Common.module.css";
import routes from "../constants/routes.json";
import Logo from "../assets/img/logobig.png";
import { Info, Transaction } from "./AppState";
import Utils from "../utils/utils";
import { parseZcashURI, ZcashURITarget } from "../utils/uris";
import { NavigateFunction, useNavigate, NavLink } from "react-router-dom";

type ExportPrivKeyModalProps = {
  modalIsOpen: boolean;
  exportedPrivKeys: string[] | null;
  closeModal: () => void;
};
const ExportPrivKeyModal = ({ modalIsOpen, exportedPrivKeys, closeModal }: ExportPrivKeyModalProps) => {
  return (
    <Modal
      isOpen={modalIsOpen}
      onRequestClose={closeModal}
      className={cstyles.modal}
      overlayClassName={cstyles.modalOverlay}
    >
      <div className={[cstyles.verticalflex].join(" ")}>
        <div className={cstyles.marginbottomlarge} style={{ textAlign: "center" }}>
          Your Wallet Private Keys
        </div>

        <div className={[cstyles.marginbottomlarge, cstyles.center].join(" ")}>
          These are all the private keys in your wallet. Please store them carefully!
        </div>

        {exportedPrivKeys && (
          <TextareaAutosize value={exportedPrivKeys.join("\n")} className={styles.exportedPrivKeys} disabled />
        )}
      </div>

      <div className={cstyles.buttoncontainer}>
        <button type="button" className={cstyles.primarybutton} onClick={closeModal}>
          Close
        </button>
      </div>
    </Modal>
  );
};

type ImportPrivKeyModalProps = {
  modalIsOpen: boolean;
  modalInput: string | null;
  setModalInput: (i: string) => void;
  doImportPrivKeys: () => void;
  closeModal: () => void;
};
const ImportPrivKeyModal = ({
  modalIsOpen,
  modalInput,
  setModalInput,
  closeModal,
  doImportPrivKeys,
}: ImportPrivKeyModalProps) => {
  return (
    <Modal
      isOpen={modalIsOpen}
      onRequestClose={closeModal}
      className={cstyles.modal}
      overlayClassName={cstyles.modalOverlay}
    >
      <div className={[cstyles.verticalflex].join(" ")}>
        <div className={cstyles.marginbottomlarge} style={{ textAlign: "center" }}>
          Import Private Keys
        </div>

        <div className={cstyles.marginbottomlarge}>
          Please paste your private or viewing keys here (transparent address or shielded address), one line per key.
        </div>

        <div className={cstyles.well} style={{ textAlign: "center" }}>
          <TextareaAutosize
            className={cstyles.inputbox}
            placeholder="Private Keys"
            value={modalInput || ""}
            onChange={(e) => setModalInput(e.target.value)}
          />
        </div>
      </div>

      <div className={cstyles.buttoncontainer}>
        <button
          type="button"
          className={cstyles.primarybutton}
          onClick={() => {
            doImportPrivKeys();
            closeModal();
          }}
        >
          Import
        </button>
        <button type="button" className={cstyles.primarybutton} onClick={closeModal}>
          Cancel
        </button>
      </div>
    </Modal>
  );
};

type PayURIModalProps = {
  modalIsOpen: boolean;
  modalInput: string | null;
  setModalInput: (i: string) => void;
  modalTitle: string;
  closeModal: () => void;
  actionButtonName: string;
  actionCallback: (u: string) => void;
};
const PayURIModal = ({
  modalIsOpen,
  modalInput,
  setModalInput,
  closeModal,
  modalTitle,
  actionButtonName,
  actionCallback,
}: PayURIModalProps) => {
  return (
    <Modal
      isOpen={modalIsOpen}
      onRequestClose={closeModal}
      className={cstyles.modal}
      overlayClassName={cstyles.modalOverlay}
    >
      <div className={[cstyles.verticalflex].join(" ")}>
        <div className={cstyles.marginbottomlarge} style={{ textAlign: "center" }}>
          {modalTitle}
        </div>

        <div className={cstyles.well} style={{ textAlign: "center" }}>
          <input
            type="text"
            className={cstyles.inputbox}
            placeholder="URI"
            value={modalInput || ""}
            onChange={(e) => setModalInput(e.target.value)}
          />
        </div>
      </div>

      <div className={cstyles.buttoncontainer}>
        {actionButtonName && (
          <button
            type="button"
            className={cstyles.primarybutton}
            onClick={() => {
              if (modalInput) {
                actionCallback(modalInput);
              }
              closeModal();
            }}
          >
            {actionButtonName}
          </button>
        )}

        <button type="button" className={cstyles.primarybutton} onClick={closeModal}>
          Close
        </button>
      </div>
    </Modal>
  );
};

type SidebarMenuItemProps = {
  name: string;
  routeName: string;
  iconname: string;
};
const SidebarMenuItem = ({ name, routeName, iconname }: SidebarMenuItemProps) => {
  const activeColorClass = styles.sidebarmenuitemactive;

  return (
    <div className={[styles.sidebarmenuitem, activeColorClass].join(" ")}>
      <NavLink to={routeName}>
        {({ isActive }) => (
          <span className={isActive ? activeColorClass : ""}>
            <i className={["fas", iconname].join(" ")} />
            &nbsp; &nbsp;
            {name}
          </span>
        )}
      </NavLink>
    </div>
  );
};

type Props = {
  info: Info;
  addresses: string[];
  transactions: Transaction[];
  setSendTo: (targets: ZcashURITarget[] | ZcashURITarget) => void;
  getPrivKeyAsString: (address: string) => string;
  importPrivKeys: (keys: string[]) => void;
  openErrorModal: (title: string, body: string | ReactElement) => void;
  closeErrorModal: () => void;
  navigate: NavigateFunction;
};

type State = {
  uriModalIsOpen: boolean;
  uriModalInputValue: string | null;
  privKeyModalIsOpen: boolean;
  privKeyInputValue: string | null;
  exportPrivKeysModalIsOpen: boolean;
  exportedPrivKeys: string[] | null;
};

class Sidebar extends PureComponent<Props, State> {
  constructor(props: Props) {
    super(props);
    this.state = {
      uriModalIsOpen: false,
      uriModalInputValue: null,
      privKeyModalIsOpen: false,
      exportPrivKeysModalIsOpen: false,
      exportedPrivKeys: null,
      privKeyInputValue: null,
    };

    this.setupMenuHandlers();
  }

  // Handle menu items
  setupMenuHandlers = async () => {
    const { info, setSendTo, openErrorModal, closeErrorModal } = this.props;
    const { testnet } = info;

    // About
    window.zecwallet.onMenuItem("about", () => {
      openErrorModal(
        "Zecwallet Fullnode",
        <div className={cstyles.verticalflex}>
          <div className={cstyles.margintoplarge}>Zecwallet Fullnode v1.8.6</div>
          <div className={cstyles.margintoplarge}>Built with Electron. Copyright (c) 2018-2023, Aditya Kulkarni.</div>
          <div className={cstyles.margintoplarge}>
            The MIT License (MIT) Copyright (c) 2018-2021 Zecwallet
            <br />
            <br />
            Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
            documentation files (the &quot;Software&quot;), to deal in the Software without restriction, including
            without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
            copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the
            following conditions:
            <br />
            <br />
            The above copyright notice and this permission notice shall be included in all copies or substantial
            portions of the Software.
            <br />
            <br />
            THE SOFTWARE IS PROVIDED &quot;AS IS&quot;, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
            NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
            NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
            IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
            USE OR OTHER DEALINGS IN THE SOFTWARE.
          </div>
        </div>
      );
    });

    // Donate button
    window.zecwallet.onMenuItem("donate", () => {
      setSendTo(
        new ZcashURITarget(
          Utils.getDonationAddress(testnet),
          Utils.getDefaultDonationAmount(testnet),
          Utils.getDefaultDonationMemo(testnet)
        )
      );

      this.props.navigate(routes.SEND);
    });

    // Pay URI
    window.zecwallet.onMenuItem("payuri", (event, uri) => {
      this.openURIModal(uri);
    });

    // Import Private Keys
    window.zecwallet.onMenuItem("import", () => {
      this.openImportPrivKeyModal(null);
    });

    // Export All Transactions
    window.zecwallet.onMenuItem("exportalltx", async () => {
      const save = await window.zecwallet.showSaveDialog(
        "Save Transactions As CSV",
        "zecwallet_transactions.csv",
        [{ name: "CSV File", extensions: ["csv"] }],
        ["showOverwriteConfirmation"]
      );

      if (save.filePath) {
        // Construct a CSV
        const { transactions } = this.props;
        const rows = transactions.flatMap((t) => {
          if (t.detailedTxns) {
            return t.detailedTxns.map((dt) => {
              const normaldate = dateformat(t.time * 1000, "mmm dd yyyy hh::MM tt");

              // Add a single quote "'" into the memo field to force interpretation as a string, rather than as a
              // formula from a rogue memo
              const escapedMemo = dt.memo ? `'${dt.memo.replace(/"/g, '""')}'` : "";

              return `${t.time},"${normaldate}","${t.txid}","${t.type}",${dt.amount},"${dt.address}","${escapedMemo}"`;
            });
          } else {
            return [];
          }
        });

        const header = [`UnixTime, Date, Txid, Type, Amount, Address, Memo`];

        try {
          await window.zecwallet.writeFile(save.filePath, header.concat(rows).join("\n"));
        } catch (err) {
          openErrorModal("Error Exporting Transactions", `${err}`);
        }
      }
    });

    // Export all private keys
    window.zecwallet.onMenuItem("exportall", async () => {
      // There might be lots of keys, so we get them serially.

      // Get all the addresses and run export key on each of them.
      const { addresses, getPrivKeyAsString } = this.props;

      // We'll do an array iteration rather than a async array.map, because there might
      // be several keys, and we don't want to hammer zcashd with 100s of RPC calls.
      const exportedPrivKeys = [];
      // eslint-disable-next-line no-restricted-syntax
      // eslint-disable-next-line no-plusplus
      for (let i = 0; i < addresses.length; i++) {
        const address = addresses[i];
        // eslint-disable-next-line no-await-in-loop
        const privKey = await getPrivKeyAsString(address);
        exportedPrivKeys.push(`${privKey} #${address}`);

        // Show a progress dialog
        openErrorModal(
          "Exporting Private Keys",
          <span>
            Exporting Private Keys
            <br />
            Please wait...({i} / {addresses.length})
          </span>
        );
      }

      closeErrorModal();
      this.setState({ exportPrivKeysModalIsOpen: true, exportedPrivKeys });
    });

    // View zcashd
    window.zecwallet.onMenuItem("zcashd", () => {
      this.props.navigate(routes.ZCASHD);
    });

    // Connect mobile app
    window.zecwallet.onMenuItem("connectmobile", () => {
      this.props.navigate(routes.CONNECTMOBILE);
    });
  };

  closeExportPrivKeysModal = () => {
    this.setState({ exportPrivKeysModalIsOpen: false, exportedPrivKeys: null });
  };

  openImportPrivKeyModal = (defaultValue: string | null) => {
    const privKeyInputValue = defaultValue || "";
    this.setState({ privKeyModalIsOpen: true, privKeyInputValue });
  };

  setImprovPrivKeyInputValue = (privKeyInputValue: string) => {
    this.setState({ privKeyInputValue });
  };

  closeImportPrivKeyModal = () => {
    this.setState({ privKeyModalIsOpen: false });
  };

  openURIModal = (defaultValue: string | null) => {
    const uriModalInputValue = defaultValue || "";
    this.setState({ uriModalIsOpen: true, uriModalInputValue });
  };

  doImportPrivKeys = () => {
    const { importPrivKeys, openErrorModal } = this.props;
    const { privKeyInputValue } = this.state;

    // eslint-disable-next-line no-control-regex
    if (privKeyInputValue) {
      // eslint-disable-next-line no-control-regex
      let keys = privKeyInputValue.split(new RegExp("[\n\r]+"));
      if (!keys || keys.length === 0) {
        openErrorModal("No Keys Imported", "No keys were specified, so none were imported");
        return;
      }

      // Filter out empty lines and clean up the private keys
      keys = keys.filter((k) => !(k.trim().startsWith("#") || k.trim().length === 0));

      // Special case.
      // Sometimes, when importing from a paperwallet or such, the key is split by newlines, and might have
      // been pasted like that. So check to see if the whole thing is one big private key
      if (Utils.isValidSaplingPrivateKey(keys.join(""))) {
        keys = [keys.join("")];
      }

      importPrivKeys(keys);
    }
  };

  setURIInputValue = (uriModalInputValue: string) => {
    this.setState({ uriModalInputValue });
  };

  closeURIModal = () => {
    this.setState({ uriModalIsOpen: false });
  };

  payURI = (uri: string) => {
    console.log(`Paying ${uri}`);
    const { openErrorModal, setSendTo } = this.props;

    const errTitle = "URI Error";
    const getErrorBody = (explain: string): ReactElement => {
      return (
        <div>
          <span>{explain}</span>
          <br />
        </div>
      );
    };

    if (!uri || uri === "") {
      openErrorModal(errTitle, getErrorBody("URI was not found or invalid"));
      return;
    }

    const parsedUri = parseZcashURI(uri);
    if (typeof parsedUri === "string") {
      openErrorModal(errTitle, getErrorBody(parsedUri));
      return;
    }

    setSendTo(parsedUri);
    this.props.navigate(routes.SEND);
  };

  render() {
    const { info } = this.props;
    const {
      uriModalIsOpen,
      uriModalInputValue,
      privKeyModalIsOpen,
      privKeyInputValue,
      exportPrivKeysModalIsOpen,
      exportedPrivKeys,
    } = this.state;

    let state = "DISCONNECTED";
    let progress = "100";
    if (info && info.version && !info.disconnected) {
      if (info.verificationProgress < 0.9999) {
        state = "SYNCING";
        progress = (info.verificationProgress * 100).toFixed(1);
      } else {
        state = "CONNECTED";
      }
    }

    return (
      <div>
        {/* Payment URI Modal */}
        <PayURIModal
          modalInput={uriModalInputValue}
          setModalInput={this.setURIInputValue}
          modalIsOpen={uriModalIsOpen}
          closeModal={this.closeURIModal}
          modalTitle="Pay URI"
          actionButtonName="Pay URI"
          actionCallback={this.payURI}
        />

        {/* Import Private Key Modal */}
        <ImportPrivKeyModal
          modalIsOpen={privKeyModalIsOpen}
          setModalInput={this.setImprovPrivKeyInputValue}
          modalInput={privKeyInputValue}
          closeModal={this.closeImportPrivKeyModal}
          doImportPrivKeys={this.doImportPrivKeys}
        />

        {/* Exported (all) Private Keys */}
        <ExportPrivKeyModal
          modalIsOpen={exportPrivKeysModalIsOpen}
          exportedPrivKeys={exportedPrivKeys}
          closeModal={this.closeExportPrivKeysModal}
        />

        <div className={[cstyles.center, styles.sidebarlogobg].join(" ")}>
          <img src={Logo} width="70" alt="logo" />
        </div>

        <div className={styles.sidebar}>
          <SidebarMenuItem name="Dashboard" routeName={routes.DASHBOARD} iconname="fa-home" />
          <SidebarMenuItem name="Send" routeName={routes.SEND} iconname="fa-paper-plane" />
          <SidebarMenuItem name="Receive" routeName={routes.RECEIVE} iconname="fa-download" />
          <SidebarMenuItem name="Transactions" routeName={routes.TRANSACTIONS} iconname="fa-list" />
          <SidebarMenuItem name="Address Book" routeName={routes.ADDRESSBOOK} iconname="fa-address-book" />
        </div>

        <div className={cstyles.center}>
          {state === "CONNECTED" && (
            <div className={[cstyles.padsmallall, cstyles.margintopsmall, cstyles.blackbg].join(" ")}>
              <i className={[cstyles.green, "fas", "fa-check"].join(" ")} />
              &nbsp; Connected
            </div>
          )}
          {state === "SYNCING" && (
            <div className={[cstyles.padsmallall, cstyles.margintopsmall, cstyles.blackbg].join(" ")}>
              <div>
                <i className={[cstyles.yellow, "fas", "fa-sync"].join(" ")} />
                &nbsp; Syncing
              </div>
              <div>{`${progress}%`}</div>
            </div>
          )}
          {state === "DISCONNECTED" && (
            <div className={[cstyles.padsmallall, cstyles.margintopsmall, cstyles.blackbg].join(" ")}>
              <i className={[cstyles.red, "fas", "fa-times-circle"].join(" ")} />
              &nbsp; Connected
            </div>
          )}
        </div>
      </div>
    );
  }
}

function WithNavigate(props: any) {
  const navigate = useNavigate();
  return <Sidebar {...props} navigate={navigate} />;
}

export default WithNavigate;
