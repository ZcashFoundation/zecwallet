/* eslint-disable max-classes-per-file */
import axios from 'axios';
import _ from 'underscore';
import hex from 'hex-string';
import { TotalBalance, AddressBalance, Transaction, RPCConfig, TxDetail, Info } from './components/AppState';
import Utils, { NO_CONNECTION } from './utils/utils';
import SentTxStore from './utils/SentTxStore';

const parseMemo = (memoHex: string): string | null => {
  if (!memoHex || memoHex.length < 2) return null;

  // First, check if this is a memo (first byte is less than 'f6' (246))
  if (parseInt(memoHex.substr(0, 2), 16) >= 246) return null;

  // Else, parse as Hex string
  const textDecoder = new TextDecoder();
  const memo = textDecoder.decode(hex.decode(memoHex));
  if (memo === '') return null;

  return memo;
};

class OpidMonitor {
  opid: string;

  fnOpenSendErrorModal: (string, string) => void;
}

export default class RPC {
  rpcConfig: RPCConfig;

  fnSetInfo: Info => void;

  fnSetTotalBalance: TotalBalance => void;

  fnSetAddressesWithBalance: (AddressBalance[]) => void;

  fnSetTransactionsList: (Transaction[]) => void;

  fnSetAllAddresses: (string[]) => void;

  fnSetZecPrice: number => void;

  fnSetDisconnected: string => void;

  opids: Set<OpidMonitor>;

  refreshTimerID: TimerID;

  opTimerID: TimerID;

  priceTimerID: TimerID;

  constructor(
    fnSetTotalBalance: TotalBalance => void,
    fnSetAddressesWithBalance: (AddressBalance[]) => void,
    fnSetTransactionsList: (Transaction[]) => void,
    fnSetAllAddresses: (string[]) => void,
    fnSetInfo: Info => void,
    fnSetZecPrice: number => void,
    fnSetDisconnected: () => void
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
      this.refreshTimerID = setTimeout(() => this.refresh(), 1000);
    }

    if (!this.opTimerID) {
      this.opTimerID = setTimeout(() => this.refreshOpStatus(), 1000);
    }

    if (!this.priceTimerID) {
      this.priceTimerID = setTimeout(() => this.getZecPrice(), 1000);
    }
  }

  setupNextFetch(lastBlockHeight: number) {
    this.refreshTimerID = setTimeout(() => this.refresh(lastBlockHeight), 60 * 1000);
  }

  static async doRPC(method: string, params: [], rpcConfig: RPCConfig) {
    const { url, username, password } = rpcConfig;

    const response = await new Promise((resolve, reject) => {
      axios(url, {
        data: {
          jsonrpc: '2.0',
          id: 'curltest',
          method,
          params
        },
        method: 'POST',
        auth: {
          username,
          password
        }
      })
        .then(r => resolve(r.data))
        .catch(err => {
          const e = { ...err };
          if (e.response && e.response.data) {
            reject(e.response.data.error.message);
          } else {
            // eslint-disable-next-line prefer-promise-reject-errors
            reject(NO_CONNECTION);
          }
        });
    });

    return response;
  }

  async refresh(lastBlockHeight: number) {
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
      console.log('Looks like stalled');
      // If it is stalled at this height after 5 seconds, we will do invalidate / reconsider
      setTimeout(async () => {
        const newHeight = await this.fetchInfo();
        if (newHeight !== 903001) {
          return;
        }

        console.log('Confirmed stalled');
        // First, clear the banned peers
        await RPC.doRPC('clearbanned', [], this.rpcConfig);
        // Then invalidate block
        await RPC.doRPC(
          'invalidateblock',
          ['00000000006e4d348d0addad1b43ae09744f9c76a0724be4a3f5e08bdb1121ac'],
          this.rpcConfig
        );
        console.log('Invalidated block');

        // And then, 2 seconds later, reconsider the block
        setTimeout(async () => {
          await RPC.doRPC(
            'reconsiderblock',
            ['00000000006e4d348d0addad1b43ae09744f9c76a0724be4a3f5e08bdb1121ac'],
            this.rpcConfig
          );
          console.log('Reconsidered block');
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
      console.log('Already have latest block, waiting for next refresh');
    }

    this.setupNextFetch(latestBlockHeight);
  }

  // Special method to get the Info object. This is used both internally and by the Loading screen
  static async getInfoObject(rpcConfig: RPCConfig) {
    const infoResult = await RPC.doRPC('getinfo', [], rpcConfig);

    const info = new Info();
    info.testnet = infoResult.result.testnet;
    info.latestBlock = infoResult.result.blocks;
    info.connections = infoResult.result.connections;
    info.version = infoResult.result.version;
    info.currencyName = info.testnet ? 'TAZ' : 'ZEC';
    info.zecPrice = null; // Setting this to null will copy over the existing price
    info.disconnected = false;

    const blkInfoResult = await RPC.doRPC('getblockchaininfo', [], rpcConfig);
    info.verificationProgress = blkInfoResult.result.verificationprogress;

    const solps = await RPC.doRPC('getnetworksolps', [], rpcConfig);
    info.solps = solps.result;

    return info;
  }

  async doImportPrivKey(key: string, rescan: boolean) {
    // Z address
    if (key.startsWith('SK') || key.startsWith('secret')) {
      try {
        const r = await RPC.doRPC('z_importkey', [key, rescan ? 'yes' : 'no'], this.rpcConfig);
        console.log(r.result);
        return '';
      } catch (err) {
        return err;
      }
    } else if (key.startsWith('zxview')) {
      try {
        const r = await RPC.doRPC('z_importviewingkey', [key, rescan ? 'yes' : 'no'], this.rpcConfig);
        console.log(r.result);
        return '';
      } catch (err) {
        return err;
      }
    } else {
      try {
        const r = await RPC.doRPC('importprivkey', [key, '', rescan], this.rpcConfig);
        console.log(r.result);
        return '';
      } catch (err) {
        return err;
      }
    }
  }

  async fetchInfo(): number {
    const info = await RPC.getInfoObject(this.rpcConfig);

    this.fnSetInfo(info);

    return info.latestBlock;
  }

  // This method will get the total balances
  async fetchTotalBalance() {
    const response = await RPC.doRPC('z_gettotalbalance', [0], this.rpcConfig);

    const balance = new TotalBalance();
    balance.total = response.result.total;
    balance.private = response.result.private;
    balance.transparent = response.result.transparent;

    this.fnSetTotalBalance(balance);
  }

  async createNewAddress(zaddress: boolean) {
    if (zaddress) {
      const newaddress = await RPC.doRPC('z_getnewaddress', ['sapling'], this.rpcConfig);

      return newaddress.result;
      // eslint-disable-next-line no-else-return
    } else {
      const newaddress = await RPC.doRPC('getnewaddress', [''], this.rpcConfig);

      return newaddress.result;
    }
  }

  async getPrivKeyAsString(address: string): string {
    let method = '';
    if (Utils.isZaddr(address)) {
      method = 'z_exportkey';
    } else if (Utils.isTransparent(address)) {
      method = 'dumpprivkey';
    }

    const response = await RPC.doRPC(method, [address], this.rpcConfig);

    return response.result;
  }

  async getViewKeyAsString(address: string): string {
    let method = '';
    if (Utils.isZaddr(address)) {
      method = 'z_exportviewingkey';
    } else {
      return '';
    }

    const response = await RPC.doRPC(method, [address], this.rpcConfig);

    return response.result;
  }

  // Fetch all addresses and their associated balances
  async fetchTandZAddressesWithBalances() {
    const zresponse = RPC.doRPC('z_listunspent', [0], this.rpcConfig);
    const tresponse = RPC.doRPC('listunspent', [0], this.rpcConfig);

    // Do the Z addresses
    // response.result has all the unspent notes.
    const unspentNotes = (await zresponse).result;
    const zgroups = _.groupBy(unspentNotes, 'address');
    const zaddresses = Object.keys(zgroups).map(address => {
      const balance = zgroups[address].reduce((prev, obj) => prev + obj.amount, 0);
      return new AddressBalance(address, Number(balance.toFixed(8)));
    });

    // Do the T addresses
    const unspentTXOs = (await tresponse).result;
    const tgroups = _.groupBy(unspentTXOs, 'address');
    const taddresses = Object.keys(tgroups).map(address => {
      const balance = tgroups[address].reduce((prev, obj) => prev + obj.amount, 0);
      return new AddressBalance(address, Number(balance.toFixed(8)));
    });

    const addresses = zaddresses.concat(taddresses);

    this.fnSetAddressesWithBalance(addresses);
  }

  // Fetch all T and Z transactions
  async fetchTandZTransactions() {
    const tresponse = await RPC.doRPC('listtransactions', [], this.rpcConfig);
    const zaddressesPromise = RPC.doRPC('z_listaddresses', [], this.rpcConfig);
    const senttxstorePromise = SentTxStore.loadSentTxns();

    const ttxlist = tresponse.result.map(tx => {
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
    const zaddresses = await zaddressesPromise;

    const alltxnsPromise = zaddresses.result.map(async zaddr => {
      // For each zaddr, get the list of incoming transactions
      const incomingTxns = await RPC.doRPC('z_listreceivedbyaddress', [zaddr, 0], this.rpcConfig);
      const txns = incomingTxns.result
        .filter(itx => !itx.change)
        .map(incomingTx => {
          return {
            address: zaddr,
            txid: incomingTx.txid,
            memo: parseMemo(incomingTx.memo),
            amount: incomingTx.amount,
            index: incomingTx.outindex
          };
        });

      return txns;
    });

    const alltxns = (await Promise.all(alltxnsPromise)).flat();

    // Now, for each tx in the array, call gettransaction
    const ztxlist = await Promise.all(
      alltxns.map(async tx => {
        const txresponse = await RPC.doRPC('gettransaction', [tx.txid], this.rpcConfig);

        const transaction = new Transaction();
        transaction.address = tx.address;
        transaction.type = 'receive';
        transaction.amount = tx.amount;
        transaction.confirmations = txresponse.result.confirmations;
        transaction.txid = tx.txid;
        transaction.time = txresponse.result.time;
        transaction.index = tx.index || 0;
        transaction.detailedTxns = [new TxDetail()];
        transaction.detailedTxns[0].address = tx.address;
        transaction.detailedTxns[0].amount = tx.amount;
        // eslint-disable-next-line no-control-regex
        transaction.detailedTxns[0].memo = tx.memo ? tx.memo.replace(/\u0000/g, '') : tx.memo;

        return transaction;
      })
    );

    // Get transactions from the sent tx store
    const sentTxns = await senttxstorePromise;

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

  // Get all Addresses, including T and Z addresses
  async fetchAllAddresses() {
    const zaddrsPromise = RPC.doRPC('z_listaddresses', [], this.rpcConfig);
    const taddrs1Promise = RPC.doRPC('getaddressesbyaccount', [''], this.rpcConfig);
    const taddrs2Promise = RPC.doRPC('listaddressgroupings', [], this.rpcConfig);
    const allZ = (await zaddrsPromise).result;
    const allT = (await taddrs1Promise).result;
    const allT2 = (await taddrs2Promise).result;
    for (let i = 0; i < allT2.length; i += 1) {
      const arrayT = allT2[i];
      for (let e = 0; e < arrayT.length; e += 1) {
        const taddr = arrayT[e][0];
        if (allT.indexOf(taddr) === -1) allT.push(taddr);
      }
    }
    this.fnSetAllAddresses(allZ.concat(allT));
  }

  // Send a transaction using the already constructed sendJson structure
  async sendTransaction(sendJson: [], fnOpenSendErrorModal: (string, string) => void): boolean {
    try {
      const opid = (await RPC.doRPC('z_sendmany', sendJson, this.rpcConfig)).result;

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
      this.opTimerID = null;
    }
  }

  async refreshOpStatus() {
    if (this.opids.size > 0) {
      // Get all the operation statuses.
      [...this.opids].map(async monitor => {
        try {
          const resultJson = await RPC.doRPC('z_getoperationstatus', [[monitor.opid]], this.rpcConfig);

          const result = resultJson.result[0];

          if (result.status === 'success') {
            const { txid } = result.result;

            monitor.fnOpenSendErrorModal(
              'Successfully Broadcast Transaction',
              `Transaction was successfully broadcast. TXID: ${txid}`
            );

            this.opids.delete(monitor);

            // And force a refresh to update the balances etc...
            this.refresh(null);
          } else if (result.status === 'failed') {
            monitor.fnOpenSendErrorModal(
              'Error Sending Transaction',
              `Opid ${monitor.opid} Failed. ${result.error.message}`
            );

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
    if (!retryCount) {
      // eslint-disable-next-line no-param-reassign
      retryCount = 0;
    }

    try {
      const response = await new Promise((resolve, reject) => {
        axios('https://api.coincap.io/v2/rates/zcash', {
          method: 'GET'
        })
          .then(r => resolve(r.data))
          .catch(err => {
            reject(err);
          });
      });

      const zecData = response.data;
      if (zecData) {
        this.fnSetZecPrice(zecData.rateUsd);
        this.setupNextZecPriceRefresh(0, 1000 * 60 * 60); // Every hour
      } else {
        this.fnSetZecPrice(null);
        let timeout = 1000 * 60; // 1 minute
        if (retryCount > 5) {
          timeout = 1000 * 60 * 60; // an hour later
        }
        this.setupNextZecPriceRefresh(retryCount + 1, timeout);
      }
    } catch (err) {
      console.log(err);
      this.fnSetZecPrice(null);
      let timeout = 1000 * 60; // 1 minute
      if (retryCount > 5) {
        timeout = 1000 * 60 * 60; // an hour later
      }
      this.setupNextZecPriceRefresh(retryCount + 1, timeout);
    }
  }
}
