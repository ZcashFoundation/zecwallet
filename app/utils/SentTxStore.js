import os from 'os';
import path from 'path';
import fs from 'fs';
import { remote } from 'electron';
import { Transaction, TxDetail } from '../components/AppState';

export default class SentTxStore {
  static locateSentTxStore() {
    if (os.platform() === 'darwin') {
      return path.join(remote.app.getPath('appData'), 'Zcash', 'senttxstore.dat');
    }

    if (os.platform() === 'linux') {
      return path.join(
        remote.app.getPath('home'),
        '.local',
        'share',
        'zec-qt-wallet-org',
        'zec-qt-wallet',
        'senttxstore.dat'
      );
    }

    return path.join(remote.app.getPath('appData'), 'Zcash', 'senttxstore.dat');
  }

  static async loadSentTxns(): Transaction[] {
    try {
      const sentTx = JSON.parse(await fs.promises.readFile(SentTxStore.locateSentTxStore()));

      return sentTx.map(s => {
        const transction = new Transaction();
        transction.type = s.type;
        transction.amount = s.amount;
        transction.address = s.from;
        transction.txid = s.txid;
        transction.time = s.datetime;
        transction.detailedTxns = [new TxDetail()];
        transction.detailedTxns[0].address = s.address;
        transction.detailedTxns[0].amount = s.amount;
        transction.detailedTxns[0].memo = s.memo;

        return transction;
      });
    } catch (err) {
      // If error for whatever reason (most likely, file not found), just return an empty array
      return [];
    }
  }
}
