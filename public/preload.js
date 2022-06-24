// All of the Node.js APIs are available in the preload process.
// It has the same sandbox as a Chrome extension.
const { clipboard, contextBridge, ipcRenderer, shell } = require("electron");

const os = require("os");
const path = require("path");
const fs = require("fs");
const ini = require("ini");

const locateZcashConf = async () => {
  if (os.platform() === "darwin") {
    return path.join(
      await ipcRenderer.invoke("getAppPath", "appData"),
      "Zcash",
      "zcash.conf"
    );
  }

  if (os.platform() === "linux") {
    return path.join(
      await ipcRenderer.invoke("getAppPath", "home"),
      ".zcash",
      "zcash.conf"
    );
  }

  return path.join(
    await ipcRenderer.invoke("getAppPath", "appData"),
    "Zcash",
    "zcash.conf"
  );
};

const loadZcashConf = async () => {
  // Load the RPC config from zcash.conf file
  const zcashLocation = await locateZcashConf();
  let confValues;
  try {
    confValues = ini.parse(
      await fs.promises.readFile(zcashLocation, { encoding: "utf-8" })
    );
  } catch (err) {
    console.log(
      `Could not find zcash.conf at ${zcashLocation}. This is a bug, please file an issue with Zecwallet`
    );
    return;
  }

  // Get the username and password
  const rpcConfig = {};
  rpcConfig.username = confValues.rpcuser;
  rpcConfig.password = confValues.rpcpassword;

  const isTestnet = (confValues.testnet && confValues.testnet === "1") || false;
  const server = confValues.rpcbind || "127.0.0.1";
  const port = confValues.rpcport || (isTestnet ? "18232" : "8232");
  rpcConfig.url = `http://${server}:${port}`;

  return rpcConfig;
};

// As an example, here we use the exposeInMainWorld API to expose the browsers
// and node versions to the main window.
// They'll be accessible at "window.versions".
process.once("loaded", () => {
  contextBridge.exposeInMainWorld("versions", process.versions);
  contextBridge.exposeInMainWorld("zecwallet", {
    onMenuItem: (name, callback) => {
      ipcRenderer.on(name, callback);
    },
    pathJoin: (dir, filename) => {
      return path.join(dir, filename);
    },
    confDir: async () => {
      return path.dirname(await locateZcashConf());
    },
    rpcconfig: async () => {
      return loadZcashConf();
    },
    doRPC: async (config, method, params) => {
      return ipcRenderer.invoke("doRPC_IPC", config, method, params);
    },
    getZecPrice: () => {
      return ipcRenderer.invoke("getZecPrice_IPC");
    },
    showSaveDialog: (title, defaultPath, filters, properties) => {
      return ipcRenderer.invoke(
        "showSaveDialog_IPC",
        title,
        defaultPath,
        filters,
        properties
      );
    },
    writeFile: (filename, content) => {
      return ipcRenderer.invoke("writefile_IPC", filename, content);
    },
    readFile: (filename) => {
      return ipcRenderer.invoke("readfile_IPC", filename);
    },
    copyToClipboard: (text) => {
      clipboard.writeText(text);
    },
    openLink: (linkURL) => {
      shell.openExternal(linkURL);
    },
  });
});
