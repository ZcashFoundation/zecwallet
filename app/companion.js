import AppState from './components/AppState';
import Utils from './utils/utils';

/* eslint-disable class-methods-use-this */
const WebSocket = window.require('ws');

export default class CompanionAppListener {
  wss = null;

  fnGetState: () => AppState = null;

  constructor(fnGetSate: () => AppState) {
    this.fnGetState = fnGetSate;
  }

  setUp() {
    this.wss = new WebSocket.Server({ port: 7070 });

    this.wss.on('connection', ws => {
      ws.on('message', message => {
        console.log(`Received message => ${message}`);
        const cmd = JSON.parse(message);

        if (cmd.command === 'getInfo') {
          const response = this.doGetInfo();
          ws.send(response);
        } else if (cmd.command === 'getTransactions') {
          const response = this.doGetTransactions();
          ws.send(response);
        }
      });
    });
  }

  doGetInfo(): any {
    const appState = this.fnGetState();

    const saplingAddress = appState.addresses.find(a => Utils.isSapling(a));
    const tAddress = appState.addresses.find(a => Utils.isTransparent(a));
    const balance = parseFloat(appState.totalBalance.total);
    const maxspendable = parseFloat(appState.totalBalance.total);
    const maxzspendable = parseFloat(appState.totalBalance.private);
    const tokenName = appState.info.currencyName;
    const zecprice = parseFloat(appState.info.zecPrice);

    const resp = {
      version: 1.0,
      command: 'getInfo',
      saplingAddress,
      tAddress,
      balance,
      maxspendable,
      maxzspendable,
      tokenName,
      zecprice,
      serverversion: '0.9.2'
    };

    return JSON.stringify(resp);
  }

  doGetTransactions(): any {
    const appState = this.fnGetState();

    let txlist = [];
    if (appState.transactions) {
      // Get only the last 20 txns
      txlist = appState.transactions.slice(0, 20).map(t => {
        let memo = t.detailedTxns && t.detailedTxns.length > 0 ? t.detailedTxns[0].memo : '';
        if (memo) {
          memo = memo.trimRight();
        } else {
          memo = '';
        }

        const txResp = {
          type: t.type,
          datetime: t.time,
          amount: t.amount.toFixed(8),
          txid: t.txid,
          address: t.address,
          memo,
          confirmations: t.confirmations
        };

        return txResp;
      });
    }

    const resp = {
      version: 1.0,
      command: 'getTransactions',
      transactions: txlist
    };

    return JSON.stringify(resp);
  }
}
