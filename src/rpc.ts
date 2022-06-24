/* eslint-disable max-classes-per-file */
import _ from "underscore";
import {
  TotalBalance,
  AddressBalance,
  Transaction,
  RPCConfig,
  TxDetail,
  Info,
  AddressType,
  AddressDetail,
} from "./components/AppState";
import Utils from "./utils/utils";
import SentTxStore from "./utils/SentTxStore";

const parseMemo = (memoHex: string): string | null => {
  if (!memoHex || memoHex.length < 2) return null;

  // First, check if this is a memo (first byte is less than 'f6' (246))
  if (parseInt(memoHex.substr(0, 2), 16) >= 246) return null;

  // Else, parse as Hex string
  const textDecoder = new TextDecoder();
  const memo = textDecoder.decode(Utils.hexStringToByte(memoHex));
  if (memo === "") return null;

  return memo;
};

class OpidMonitor {
  opid: string;
  fnOpenSendErrorModal?: (title: string, msg: string) => void;

  constructor() {
    this.opid = "";
  }
}

type RPCResult = {
  result: any;
};

export default class RPC {
  rpcConfig?: RPCConfig;

  fnSetInfo: (info: Info) => void;
  fnSetTotalBalance: (totalBalance: TotalBalance) => void;
  fnSetAddressesWithBalance: (ab: AddressBalance[]) => void;
  fnSetTransactionsList: (txns: Transaction[]) => void;
  fnSetAllAddresses: (allAddresses: AddressDetail[]) => void;
  fnSetZecPrice: (price: number) => void;
  fnSetDisconnected: (msg: string) => void;

  opids: Set<OpidMonitor>;

  refreshTimerID?: NodeJS.Timeout;
  opTimerID?: NodeJS.Timeout;
  priceTimerID?: NodeJS.Timeout;

  constructor(
    fnSetInfo: (info: Info) => void,
    fnSetTotalBalance: (totalBalance: TotalBalance) => void,
    fnSetAddressesWithBalance: (ab: AddressBalance[]) => void,
    fnSetTransactionsList: (txns: Transaction[]) => void,
    fnSetAllAddresses: (allAddresses: AddressDetail[]) => void,
    fnSetZecPrice: (price: number) => void,
    fnSetDisconnected: (msg: string) => void
  ) {
    this.fnSetTotalBalance = fnSetTotalBalance;
    this.fnSetAddressesWithBalance = fnSetAddressesWithBalance;
    this.fnSetTransactionsList = fnSetTransactionsList;
    this.fnSetAllAddresses = fnSetAllAddresses;
    this.fnSetInfo = fnSetInfo;
    this.fnSetZecPrice = fnSetZecPrice;
    this.fnSetDisconnected = fnSetDisconnected;

    this.opids = new Set();
  }

  async configure(rpcConfig: RPCConfig) {
    this.rpcConfig = rpcConfig;

    if (!this.refreshTimerID) {
      this.refreshTimerID = setTimeout(() => this.refresh(null), 1000);
    }

    if (!this.opTimerID) {
      this.opTimerID = setTimeout(() => this.refreshOpStatus(), 1000);
    }

    if (!this.priceTimerID) {
      this.priceTimerID = setTimeout(() => this.getZecPrice(0), 1000);
    }
  }

  setupNextFetch(lastBlockHeight: number) {
    this.refreshTimerID = setTimeout(
      () => this.refresh(lastBlockHeight),
      60 * 1000
    );
  }

  static async doRPC(
    method: string,
    params: any[],
    rpcConfig?: RPCConfig
  ): Promise<RPCResult> {
    if (!rpcConfig) {
      throw new Error("No RPC Config");
    }

    const response = await window.zecwallet.doRPC(rpcConfig, method, params);
    return response as RPCResult;
  }

  async refresh(lastBlockHeight: number | null) {
    let latestBlockHeight;
    try {
      latestBlockHeight = await this.fetchInfo();
    } catch (err) {
      // If we caught an error, there's something wrong with the connection.
      this.fnSetDisconnected(`${err}`);
      return;
    }

    // Temporary fix for zcashd stalls at 903001 issue (https://github.com/zcash/zcash/issues/4620)
    if (latestBlockHeight === 903001) {
      console.log("Looks like stalled");
      // If it is stalled at this height after 5 seconds, we will do invalidate / reconsider
      setTimeout(async () => {
        const newHeight = await this.fetchInfo();
        if (newHeight !== 903001) {
          return;
        }

        console.log("Confirmed stalled");
        // First, clear the banned peers
        await RPC.doRPC("clearbanned", [], this.rpcConfig);
        // Then invalidate block
        await RPC.doRPC(
          "invalidateblock",
          ["00000000006e4d348d0addad1b43ae09744f9c76a0724be4a3f5e08bdb1121ac"],
          this.rpcConfig
        );
        console.log("Invalidated block");

        // And then, 2 seconds later, reconsider the block
        setTimeout(async () => {
          await RPC.doRPC(
            "reconsiderblock",
            [
              "00000000006e4d348d0addad1b43ae09744f9c76a0724be4a3f5e08bdb1121ac",
            ],
            this.rpcConfig
          );
          console.log("Reconsidered block");
        }, 2 * 1000);
      }, 5 * 1000);
    }

    if (!lastBlockHeight || lastBlockHeight < latestBlockHeight) {
      try {
        const balP = this.fetchTotalBalance();
        const abP = this.fetchTandZAddressesWithBalances();
        const txns = this.fetchTandZTransactions();
        const addrs = this.fetchAllAddresses();

        await balP;
        await abP;
        await txns;
        await addrs;

        // All done, set up next fetch
        console.log(`Finished full refresh at ${latestBlockHeight}`);
      } catch (err) {
        // If we caught an error, there's something wrong with the connection.
        this.fnSetDisconnected(`${err}`);
        return;
      }
    } else {
      // Still at the latest block
      console.log("Already have latest block, waiting for next refresh");
    }

    this.setupNextFetch(latestBlockHeight);
  }

  // Special method to get the Info object. This is used both internally and by the Loading screen
  static async getInfoObject(rpcConfig?: RPCConfig) {
    const infoResult = await RPC.doRPC("getinfo", [], rpcConfig);

    const info = new Info();
    info.testnet = infoResult.result.testnet;
    info.latestBlock = infoResult.result.blocks;
    info.connections = infoResult.result.connections;
    info.version = infoResult.result.version;
    info.currencyName = info.testnet ? "TAZ" : "ZEC";
    info.zecPrice = null; // Setting this to null will copy over the existing price
    info.disconnected = false;

    const blkInfoResult = await RPC.doRPC("getblockchaininfo", [], rpcConfig);
    info.verificationProgress = blkInfoResult.result.verificationprogress;

    const solps = await RPC.doRPC("getnetworksolps", [], rpcConfig);
    info.solps = solps.result;

    return info;
  }

  async doImportPrivKey(key: string, rescan: boolean) {
    // Z address
    if (key.startsWith("SK") || key.startsWith("secret")) {
      try {
        const r = await RPC.doRPC(
          "z_importkey",
          [key, rescan ? "yes" : "no"],
          this.rpcConfig
        );
        console.log(r.result);
        return "";
      } catch (err) {
        return err;
      }
    } else if (key.startsWith("zxview")) {
      try {
        const r = await RPC.doRPC(
          "z_importviewingkey",
          [key, rescan ? "yes" : "no"],
          this.rpcConfig
        );
        console.log(r.result);
        return "";
      } catch (err) {
        return err;
      }
    } else {
      try {
        const r = await RPC.doRPC(
          "importprivkey",
          [key, "", rescan],
          this.rpcConfig
        );
        console.log(r.result);
        return "";
      } catch (err) {
        return err;
      }
    }
  }

  async fetchInfo(): Promise<number> {
    const info = await RPC.getInfoObject(this.rpcConfig);

    this.fnSetInfo(info);

    return info.latestBlock;
  }

  // This method will get the total balances
  async fetchTotalBalance() {
    const response = await RPC.doRPC("z_gettotalbalance", [0], this.rpcConfig);

    const balance = new TotalBalance();
    balance.total = parseFloat(response.result.total);
    balance.private = parseFloat(response.result.private);
    balance.transparent = parseFloat(response.result.transparent);

    this.fnSetTotalBalance(balance);
  }

  async createNewAddress(type: AddressType): Promise<string> {
    if (type === AddressType.unified) {
      // First make sure that at least one account has been created.
      const accounts = await RPC.doRPC("z_listaccounts", [], this.rpcConfig);
      if (accounts.result.length === 0) {
        // Create a new account
        const newAcct = await RPC.doRPC("z_getnewaccount", [], this.rpcConfig);

        console.log(`New account : ${newAcct.result}`);
      }

      const newaddress = await RPC.doRPC(
        "z_getaddressforaccount",
        [0],
        this.rpcConfig
      );

      return newaddress.result.address;
    } else if (type === AddressType.sapling) {
      const newaddress = await RPC.doRPC(
        "z_getnewaddress",
        ["sapling"],
        this.rpcConfig
      );

      return newaddress.result;
    } else {
      const newaddress = await RPC.doRPC("getnewaddress", [""], this.rpcConfig);

      return newaddress.result;
    }
  }

  async getPrivKeyAsString(address: string): Promise<string> {
    let method = "";
    if (Utils.isZaddr(address)) {
      method = "z_exportkey";
    } else if (Utils.isTransparent(address)) {
      method = "dumpprivkey";
    }

    const response = await RPC.doRPC(method, [address], this.rpcConfig);

    return response.result;
  }

  async getViewKeyAsString(address: string): Promise<string> {
    let method = "";
    if (Utils.isZaddr(address)) {
      method = "z_exportviewingkey";
    } else {
      return "";
    }

    const response = await RPC.doRPC(method, [address], this.rpcConfig);

    return response.result;
  }

  // Fetch all addresses and their associated balances
  async fetchTandZAddressesWithBalances() {
    const zresponse = RPC.doRPC("z_listunspent", [0], this.rpcConfig);
    const tresponse = RPC.doRPC("listunspent", [0], this.rpcConfig);
    const uresponse = RPC.doRPC("z_getbalanceforaccount", [0], this.rpcConfig);

    // Do the Z addresses
    // response.result has all the unspent notes.
    const unspentNotes = (await zresponse).result;
    const zgroups = _.groupBy(unspentNotes, "address");
    const zaddresses = Object.keys(zgroups)
      .filter((address) => Utils.isSapling(address))
      .map((address) => {
        const balance = zgroups[address].reduce(
          (prev, obj) => prev + obj.amount,
          0
        );

        return new AddressBalance(address, Number(balance.toFixed(8)));
      });

    // Do the T addresses
    const unspentTXOs = (await tresponse).result;
    const tgroups = _.groupBy(unspentTXOs, "address");
    const taddresses = Object.keys(tgroups).map((address) => {
      const balance = tgroups[address].reduce(
        (prev, obj) => prev + obj.amount,
        0
      );
      return new AddressBalance(address, Number(balance.toFixed(8)));
    });

    // Do the U addresses
    const ubalances = (await uresponse).result;
    const uaddresses = new AddressBalance(
      Utils.UAStringfromAccount(0),
      ubalances.pools.orchard
        ? Utils.zatToZec(ubalances.pools.orchard.valueZat)
        : 0
    );

    const addresses = zaddresses.concat(taddresses).concat(uaddresses);

    this.fnSetAddressesWithBalance(addresses);
  }

  // Fetch all T and Z transactions
  async fetchTandZTransactions() {
    const tresponse = await RPC.doRPC("listtransactions", [], this.rpcConfig);
    const zaddressesPromise = this.getAllAddresses();
    const senttxstorePromise = SentTxStore.loadSentTxns();

    const ttxlist: Transaction[] = tresponse.result.map((tx: any) => {
      const transaction = new Transaction();
      transaction.address = tx.address;
      transaction.type = tx.category;
      transaction.amount = tx.amount;
      transaction.fee = Math.abs(tx.fee || 0);
      transaction.confirmations = tx.confirmations;
      transaction.txid = tx.txid;
      transaction.time = tx.time;
      transaction.detailedTxns = [new TxDetail()];
      transaction.detailedTxns[0].address = tx.address;
      transaction.detailedTxns[0].amount = tx.amount;

      return transaction;
    });

    // Now get Z txns
    const zaddresses = (await zaddressesPromise).filter(
      (addr) =>
        addr.type === AddressType.sapling || addr.type === AddressType.unified
    );

    const alltxns = new Array<any>();
    for (let i = 0; i < zaddresses.length; i++) {
      const zaddr = zaddresses[i];

      // For each zaddr, get the list of incoming transactions
      const incomingTxns: any = await RPC.doRPC(
        "z_listreceivedbyaddress",
        [zaddr.address, 0],
        this.rpcConfig
      );
      const txns = incomingTxns.result
        .filter((itx: any) => !itx.change)
        .map((incomingTx: any) => {
          return {
            address: zaddr.address,
            txid: incomingTx.txid,
            memo: parseMemo(incomingTx.memo),
            amount: incomingTx.amount,
            index: incomingTx.outindex,
          };
        });

      alltxns.push.apply(alltxns, txns);
    }

    // Now, for each tx in the array, call gettransaction
    const ztxlist = new Array<Transaction>();
    for (let i = 0; i < alltxns.length; i++) {
      const tx = alltxns[i];
      const txresponse: any = await RPC.doRPC(
        "gettransaction",
        [tx.txid],
        this.rpcConfig
      );

      const transaction = new Transaction();
      transaction.address = tx.address;
      transaction.type = "receive";
      transaction.amount = tx.amount;
      transaction.confirmations = txresponse.result.confirmations;
      transaction.txid = tx.txid;
      transaction.time = txresponse.result.time;
      transaction.index = tx.index || 0;
      transaction.detailedTxns = [new TxDetail()];
      transaction.detailedTxns[0].address = tx.address;
      transaction.detailedTxns[0].amount = tx.amount;
      transaction.detailedTxns[0].memo = tx.memo
        ? // eslint-disable-next-line no-control-regex
          tx.memo.replace(/\u0000/g, "")
        : tx.memo;

      ztxlist.push(transaction);
    }

    // Get transactions from the sent tx store
    const sentTxns: Transaction[] = await senttxstorePromise;

    // Now concat the t and z transactions, and call the update function again
    const alltxlist = ttxlist
      .concat(ztxlist)
      .concat(sentTxns)
      .sort((tx1, tx2) => {
        if (tx1.time && tx2.time) {
          return tx2.time - tx1.time;
        }

        return tx1.confirmations - tx2.confirmations;
      });

    this.fnSetTransactionsList(alltxlist);
  }

  async fetchAllAddresses() {
    const allAddresses = await this.getAllAddresses();
    this.fnSetAllAddresses(allAddresses);
  }

  // Get all Addresses, including T and Z addresses
  async getAllAddresses(): Promise<AddressDetail[]> {
    const allAddresses = await RPC.doRPC("listaddresses", [], this.rpcConfig);

    // This returns multiple objects, each has transparent, sapling and unified addresses
    let allT: AddressDetail[] = [];
    let allZ: AddressDetail[] = [];
    let allU: AddressDetail[] = [];

    allAddresses.result.forEach((addressSet: any) => {
      const transparent: any = addressSet["transparent"];
      if (transparent) {
        const tAddrs = transparent["addresses"];
        if (tAddrs) {
          allT = allT.concat(
            tAddrs.map(
              (taddr: string) =>
                new AddressDetail(taddr, AddressType.transparent)
            )
          );
        }

        const changeTaddrs = transparent["changeAddresses"];
        if (changeTaddrs) {
          allT = allT.concat(
            changeTaddrs.map(
              (taddr: string) =>
                new AddressDetail(taddr, AddressType.transparent)
            )
          );
        }
      }

      const saplingAddrs: any[] = addressSet["sapling"];
      if (saplingAddrs) {
        saplingAddrs.forEach((saplingA) => {
          const zAddrs: string[] = saplingA["addresses"];
          if (zAddrs) {
            allZ = allZ.concat(
              zAddrs.map(
                (zaddr: string) => new AddressDetail(zaddr, AddressType.sapling)
              )
            );
          }
        });
      }

      const unifiedAddrs: any[] = addressSet["unified"];
      if (unifiedAddrs) {
        unifiedAddrs.forEach((uaSet) => {
          const account = uaSet["account"];
          const uAddrs: any[] = uaSet["addresses"];
          if (uAddrs) {
            uAddrs.forEach((uaddr) => {
              const diversifier = uaddr["diversifier_index"];
              const ua = uaddr["address"];
              if (ua) {
                allU = allU.concat([
                  new AddressDetail(
                    ua,
                    AddressType.unified,
                    account,
                    diversifier
                  ),
                ]);
              }
            });
          }
        });
      }
    });

    return allU.concat(allZ.concat(allT));
  }

  // Send a transaction using the already constructed sendJson structure
  async sendTransaction(
    sendJson: any[],
    fnOpenSendErrorModal: (title: string, msg: string) => void
  ): Promise<boolean> {
    try {
      const res: any = await RPC.doRPC("z_sendmany", sendJson, this.rpcConfig);
      const opid: string = res.result;

      const monitor = new OpidMonitor();
      monitor.opid = opid;
      monitor.fnOpenSendErrorModal = fnOpenSendErrorModal;

      this.addOpidToMonitor(monitor);

      return true;
    } catch (err) {
      // TODO Show a modal with the error
      console.log(`Error sending Tx: ${err}`);
      throw err;
    }
  }

  // Start monitoring the given opid
  async addOpidToMonitor(monitor: OpidMonitor) {
    this.opids.add(monitor);
    this.refreshOpStatus();
  }

  setupNextOpidSatusFetch() {
    if (this.opids.size > 0) {
      this.opTimerID = setTimeout(() => this.refreshOpStatus(), 2000); // 2 sec
    } else {
      this.opTimerID = undefined;
    }
  }

  async refreshOpStatus() {
    if (this.opids.size > 0) {
      // Get all the operation statuses.
      Array.from(this.opids).map(async (monitor: OpidMonitor) => {
        try {
          const resultJson: any = await RPC.doRPC(
            "z_getoperationstatus",
            [[monitor.opid]],
            this.rpcConfig
          );

          const result = resultJson.result[0];

          if (result.status === "success") {
            const { txid } = result.result;

            if (monitor.fnOpenSendErrorModal) {
              monitor.fnOpenSendErrorModal(
                "Successfully Broadcast Transaction",
                `Transaction was successfully broadcast. TXID: ${txid}`
              );
            }

            this.opids.delete(monitor);

            // And force a refresh to update the balances etc...
            this.refresh(null);
          } else if (result.status === "failed") {
            if (monitor.fnOpenSendErrorModal) {
              monitor.fnOpenSendErrorModal(
                "Error Sending Transaction",
                `Opid ${monitor.opid} Failed. ${result.error.message}`
              );
            }
            this.opids.delete(monitor);
          }
        } catch (err) {
          // If we can't get a response for this OPID, then just forget it and move on
          this.opids.delete(monitor);
        }
      });
    }

    this.setupNextOpidSatusFetch();
  }

  setupNextZecPriceRefresh(retryCount: number, timeout: number) {
    // Every hour
    this.priceTimerID = setTimeout(() => this.getZecPrice(retryCount), timeout);
  }

  async getZecPrice(retryCount: number) {
    try {
      // @ts-ignore
      const response: any = await window.zecwallet.getZecPrice();

      const zecData = response.data;
      if (zecData) {
        this.fnSetZecPrice(zecData.rateUsd);
        this.setupNextZecPriceRefresh(0, 1000 * 60 * 60); // Every hour
      } else {
        this.fnSetZecPrice(0);
        let timeout = 1000 * 60; // 1 minute
        if (retryCount > 5) {
          timeout = 1000 * 60 * 60; // an hour later
        }
        this.setupNextZecPriceRefresh(retryCount + 1, timeout);
      }
    } catch (err) {
      console.log(err);
      this.fnSetZecPrice(0);
      let timeout = 1000 * 60; // 1 minute
      if (retryCount > 5) {
        timeout = 1000 * 60 * 60; // an hour later
      }
      this.setupNextZecPriceRefresh(retryCount + 1, timeout);
    }
  }
}
