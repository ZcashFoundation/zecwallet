import { AddressBookEntry } from "../components/AppState";

// Utility class to save / read the address book.
export default class AddressbookImpl {
  static async getFileName() {
    const fileName = window.zecwallet.pathJoin(
      await window.zecwallet.confDir(),
      "AddressBook.json"
    );

    return fileName;
  }

  // Write the address book to disk
  static async writeAddressBook(ab: AddressBookEntry[]) {
    const fileName = await this.getFileName();

    await window.zecwallet.writeFile(fileName, JSON.stringify(ab));
  }

  // Read the address book
  static async readAddressBook(): Promise<AddressBookEntry[]> {
    try {
      const fileName = await this.getFileName();
      return await window.zecwallet.readFile(fileName);
    } catch (err) {
      // File probably doesn't exist, so return nothing
      console.log(err);
      return [];
    }
  }
}
