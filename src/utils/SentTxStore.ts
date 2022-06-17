import { Transaction, TxDetail } from "../components/AppState";

export default class SentTxStore {
  static async locateSentTxStore() {
    return window.zecwallet.pathJoin(
      await window.zecwallet.confDir(),
      "senttxstore.dat"
    );
  }

  static async loadSentTxns(): Promise<Transaction[]> {
    try {
      const sentTx: any[] = await window.zecwallet.readFile(
        await this.locateSentTxStore()
      );

      return sentTx.map((s: any) => {
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
