/* eslint-disable max-classes-per-file */

import { ErrorModalData } from "./ErrorModal";

export enum AddressType {
  transparent,
  sapling,
  unified,
}

export class TotalBalance {
  // Total t address, confirmed
  transparent: number;

  // Total t address, unconfirmed
  unconfirmedTransparent: number;

  // Total private, confirmed + spendable
  private: number;

  // Total private, unconfirmed or not spendable
  unconfirmedPrivate: number;

  // Total confirmed + spendable
  total: number;

  constructor() {
    this.transparent = 0;
    this.unconfirmedPrivate = 0;
    this.unconfirmedTransparent = 0;
    this.private = 0;
    this.total = 0;
  }
}

export class AddressBalance {
  address: string;

  balance: number;

  constructor(address: string, balance: number) {
    this.address = address;
    this.balance = balance;
  }
}

export class AddressBookEntry {
  label: string;
  address: string;

  constructor(label: string, address: string) {
    this.label = label;
    this.address = address;
  }
}

export class TxDetail {
  address: string;
  amount: string;
  memo: string | null;

  constructor() {
    this.address = "";
    this.amount = "";
    this.memo = null;
  }
}

// List of transactions. TODO: Handle memos, multiple addresses etc...
export class Transaction {
  type: string;
  address: string;
  amount: number;
  confirmations: number;
  txid: string;
  index: number;
  fee: number;
  time: number;

  detailedTxns: TxDetail[];

  constructor() {
    this.type = "";
    this.address = "";
    this.amount = 0;
    this.confirmations = 0;
    this.txid = "";
    this.index = 0;
    this.fee = 0;
    this.time = 0;

    this.detailedTxns = [];
  }
}

export class ToAddr {
  id: number;
  to: string;
  amount: number;
  memo: string | null;

  constructor(id: number) {
    this.id = id;

    this.to = "";
    this.amount = 0;
    this.memo = null;
  }
}

export class SendPageState {
  fromaddr: string;
  privacyLevel: string;
  toaddrs: ToAddr[];

  constructor() {
    this.fromaddr = "";
    this.privacyLevel = "FullPrivacy";
    this.toaddrs = [];
  }

  clone(): SendPageState {
    const newState = new SendPageState();
    newState.fromaddr = this.fromaddr;
    newState.privacyLevel = this.privacyLevel;
    newState.toaddrs = this.toaddrs;

    return newState;
  }
}

export class ReceivePageState {
  // A newly created address to show by default
  newAddress: string;

  // The key used for the receive page component.
  // Increment to force re-render
  rerenderKey: number;

  constructor() {
    this.newAddress = "";
    this.rerenderKey = 0;
  }
}

export class RPCConfig {
  url: string;
  username: string;
  password: string;

  constructor() {
    this.username = "";
    this.password = "";
    this.url = "";
  }
}

export class Info {
  testnet: boolean;
  latestBlock: number;
  connections: number;
  version: number;
  verificationProgress: number;
  currencyName: string;
  solps: number;
  zecPrice: number | null;
  disconnected: boolean;

  constructor() {
    this.testnet = false;
    this.latestBlock = 0;
    this.connections = 0;
    this.version = 0;
    this.verificationProgress = 0;
    this.currencyName = "";
    this.solps = 0;
    this.zecPrice = null;
    this.disconnected = true;
  }
}

export class AddressDetail {
  address: string;
  type: AddressType;
  account?: number;
  diversifier?: number;

  constructor(
    address: string,
    type: AddressType,
    account?: number,
    diversifier?: number
  ) {
    this.address = address;
    this.type = type;
    this.account = account;
    this.diversifier = diversifier;
  }
}

// eslint-disable-next-line max-classes-per-file
export default class AppState {
  // The total confirmed and unconfirmed balance in this wallet
  totalBalance: TotalBalance;

  // The list of all t and z addresses that have a current balance. That is, the list of
  // addresses that have a (confirmed or unconfirmed) UTXO or note pending.
  addressesWithBalance: AddressBalance[];

  // A map type that contains address -> privatekey/viewkey mapping, for display on the receive page
  // This mapping is ephemeral, and will disappear when the user navigates away.
  addressPrivateKeys: Map<string, string>;
  addressViewKeys: Map<string, string>;

  // List of all addresses in the wallet, including change addresses and addresses
  // that don't have any balance or are unused
  addresses: AddressDetail[];

  // List of Address / Label pairs
  addressBook: AddressBookEntry[];

  // List of all T and Z transactions
  transactions: Transaction[];

  // The state of the send page, as the user constructs a transaction
  sendPageState: SendPageState;

  // Any state for the receive page
  receivePageState: ReceivePageState;

  // Error modal Data
  errorModalData: ErrorModalData;

  // The Current configuration of the RPC params
  rpcConfig: RPCConfig;

  // getinfo and getblockchaininfo result
  info: Info;

  constructor() {
    this.totalBalance = new TotalBalance();
    this.addressesWithBalance = [];
    this.addressPrivateKeys = new Map();
    this.addressViewKeys = new Map();
    this.addresses = [];
    this.addressBook = [];
    this.transactions = [];
    this.sendPageState = new SendPageState();
    this.receivePageState = new ReceivePageState();
    this.errorModalData = new ErrorModalData();

    this.rpcConfig = new RPCConfig();
    this.info = new Info();
  }
}
