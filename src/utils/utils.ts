/* eslint-disable no-unused-vars */
/* eslint-disable no-else-return */
/* eslint-disable no-plusplus */
export const NO_CONNECTION: string = "Could not connect to zcashd";

export default class Utils {
  static UAStringfromAccount(account: number): string {
    return `UA account ${account}`;
  }

  static UAAccountfromString(account_str: string): number {
    const matches = account_str.match(new RegExp("^UA account ([0-9]+)$"));
    if (matches && matches.length === 2) {
      return parseInt(matches[1]);
    } else {
      return 0;
    }
  }

  static isUnified(addr: string): boolean {
    if (!addr) return false;
    return addr.startsWith("u");
  }

  static isSapling(addr: string): boolean {
    if (!addr) return false;
    return (
      new RegExp("^z[a-z0-9]{77}$").test(addr) ||
      new RegExp("^ztestsapling[a-z0-9]{76}$").test(addr)
    );
  }

  static isSprout(addr: string): boolean {
    if (!addr) return false;
    return new RegExp("^z[a-zA-Z0-9]{94}$").test(addr);
  }

  static isZaddr(addr: string): boolean {
    if (!addr) return false;
    return Utils.isSapling(addr) || Utils.isSprout(addr);
  }

  static isTransparent(addr: string): boolean {
    if (!addr) return false;
    return new RegExp("^t[a-zA-Z0-9]{34}$").test(addr);
  }

  static isValidSaplingPrivateKey(key: string): boolean {
    return (
      new RegExp("^secret-extended-key-test[0-9a-z]{278}$").test(key) ||
      new RegExp("^secret-extended-key-main[0-9a-z]{278}$").test(key)
    );
  }

  static zatToZec(amount: number): number {
    if (amount && amount > 0) {
      return amount / 10 ** 8;
    } else {
      return 0;
    }
  }

  // Convert to max 8 decimal places, and remove trailing zeros
  static maxPrecision(v: number): string {
    return v.toFixed(8);
  }

  static maxPrecisionTrimmed(v: number): string {
    let s = Utils.maxPrecision(v);
    if (!s) {
      return s;
    }

    while (s.indexOf(".") >= 0 && s.substr(s.length - 1, 1) === "0") {
      s = s.substr(0, s.length - 1);
    }

    if (s.substr(s.length - 1) === ".") {
      s = s.substr(0, s.length - 1);
    }

    return s;
  }

  static splitZecAmountIntoBigSmall(zecValue: number) {
    if (!zecValue) {
      return { bigPart: zecValue, smallPart: "" };
    }

    let bigPart = Utils.maxPrecision(zecValue);
    let smallPart = "";

    if (bigPart.indexOf(".") >= 0) {
      const decimalPart = bigPart.substr(bigPart.indexOf(".") + 1);
      if (decimalPart.length > 4) {
        smallPart = decimalPart.substr(4);
        bigPart = bigPart.substr(0, bigPart.length - smallPart.length);

        // Pad the small part with trailing 0s
        while (smallPart.length < 4) {
          smallPart += "0";
        }
      }
    }

    if (smallPart === "0000") {
      smallPart = "";
    }

    return { bigPart, smallPart };
  }

  static splitStringIntoChunks(s: string, numChunks: number) {
    if (numChunks > s.length) return [s];
    if (s.length < 16) return [s];

    const chunkSize = Math.round(s.length / numChunks);
    const chunks = [];
    for (let i = 0; i < numChunks - 1; i++) {
      chunks.push(s.substr(i * chunkSize, chunkSize));
    }
    // Last chunk might contain un-even length
    chunks.push(s.substr((numChunks - 1) * chunkSize));

    return chunks;
  }

  static nextToAddrID: number = 0;

  static getNextToAddrID(): number {
    // eslint-disable-next-line no-plusplus
    return Utils.nextToAddrID++;
  }

  static getDefaultFee(height: number): number {
    if (height >= 1080000) {
      return 0.00001;
    } else {
      return 0.0001;
    }
  }

  static getDonationAddress(testnet: boolean): string {
    if (testnet) {
      return "ztestsapling1wn6889vznyu42wzmkakl2effhllhpe4azhu696edg2x6me4kfsnmqwpglaxzs7tmqsq7kudemp5";
    } else {
      return "zs1gv64eu0v2wx7raxqxlmj354y9ycznwaau9kduljzczxztvs4qcl00kn2sjxtejvrxnkucw5xx9u";
    }
  }

  static getDefaultDonationAmount(testnet: boolean): number {
    return 0.1;
  }

  static getDefaultDonationMemo(testnet: boolean): string {
    return "Thanks for supporting Zecwallet!";
  }

  static getZecToUsdString(
    price: number | null,
    zecValue: number | null
  ): string {
    if (!price || !zecValue) {
      return "USD --";
    }

    return `USD ${(price * zecValue).toFixed(2)}`;
  }

  static hexStringToByte(str: string): Uint8Array {
    if (!str) {
      return new Uint8Array();
    }

    var a = [];
    for (var i = 0, len = str.length; i < len; i += 2) {
      a.push(parseInt(str.substr(i, 2), 16));
    }

    return new Uint8Array(a);
  }

  static bytesToHexString(bytes: Uint8Array): string {
    if (!bytes) {
      return "";
    }

    bytes = new Uint8Array(bytes);
    var hexBytes = [];

    for (var i = 0; i < bytes.length; ++i) {
      var byteString = bytes[i].toString(16);
      if (byteString.length < 2) {
        byteString = "0" + byteString;
      }
      hexBytes.push(byteString);
    }
    return hexBytes.join("");
  }
}
