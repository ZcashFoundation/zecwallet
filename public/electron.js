// Module to control the application lifecycle and the native browser window.
const { app, BrowserWindow, protocol, ipcMain, dialog } = require("electron");

const path = require("path");
const url = require("url");
const axios = require("axios");
const fs = require("fs");
const MenuBuilder = require("./menu");

ipcMain.handle("getAppPath", (_, pathType) => {
  return app.getPath(pathType);
});

ipcMain.handle("pathJoin_IPC", (_, dir, filename) => {
  return path.join(dir, filename);
});

ipcMain.handle("doRPC_IPC", async (_, config, method, params) => {
  const response = await new Promise((resolve, reject) => {
    axios(config.url, {
      data: {
        jsonrpc: "2.0",
        id: "curltest",
        method: method,
        params: params,
      },
      method: "POST",
      auth: {
        username: config.username,
        password: config.password,
      },
    })
      .then((r) => resolve(r.data))
      .catch((err) => {
        const e = { ...err };

        console.log(`Error calling ${method} with ${JSON.stringify(params)}`);

        if (e.response && e.response.data && e.response.data.error) {
          console.log(JSON.stringify(e.response.data.error));
        }
        console.log(
          `Caught error: ${e.response} - ${
            e.response ? e.response.data.error : ""
          }`
        );

        if (e.response && e.response.data && e.response.data.error) {
          reject(JSON.stringify(e.response.data.error));
        } else {
          reject("NO_CONNECTION");
        }
      });
  });

  return response;
});

ipcMain.handle("getZecPrice_IPC", async () => {
  const response = await new Promise((resolve, reject) => {
    axios("https://api.coincap.io/v2/rates/zcash", {
      method: "GET",
    })
      .then((r) => resolve(r.data))
      .catch((err) => {
        reject(err);
      });
  });

  return response;
});

ipcMain.handle(
  "showSaveDialog_IPC",
  async (_, title, defaultPath, filters, properties) => {
    return dialog.showSaveDialog({ title, defaultPath, filters, properties });
  }
);

ipcMain.handle("writefile_IPC", async (_, filename, content) => {
  return fs.promises.writeFile(filename, content);
});

ipcMain.handle("readfile_IPC", async (_, filename) => {
  return JSON.parse(await fs.promises.readFile(filename));
});

// Create the native browser window.
function createWindow() {
  const width = app.isPackaged ? 1200 : 2400;

  const mainWindow = new BrowserWindow({
    width: width,
    height: 800,
    // Set the path of an additional "preload" script that can be used to
    // communicate between node-land and browser-land.
    webPreferences: {
      preload: path.join(__dirname, "preload.js"),
    },
  });

  // In production, set the initial browser path to the local bundle generated
  // by the Create React App build process.
  // In development, set it to localhost to allow live/hot-reloading.
  const appURL = app.isPackaged
    ? url.format({
        pathname: path.join(__dirname, "index.html"),
        protocol: "file:",
        slashes: true,
      })
    : "http://127.0.0.1:3000";
  mainWindow.loadURL(appURL);

  // Automatically open Chrome's DevTools in development mode.
  // if (!app.isPackaged) {
  mainWindow.webContents.openDevTools();
  // }

  const menuBuilder = new MenuBuilder(mainWindow);
  menuBuilder.buildMenu();
}

// Setup a local proxy to adjust the paths of requested files when loading
// them from the local production bundle (e.g.: local fonts, etc...).
function setupLocalFilesNormalizerProxy() {
  protocol.registerHttpProtocol(
    "file",
    (request, callback) => {
      const url = request.url.substr(8);
      callback({ path: path.normalize(`${__dirname}/${url}`) });
    },
    (error) => {
      if (error) console.error("Failed to register protocol");
    }
  );
}

// This method will be called when Electron has finished its initialization and
// is ready to create the browser windows.
// Some APIs can only be used after this event occurs.
app.whenReady().then(() => {
  createWindow();
  setupLocalFilesNormalizerProxy();

  app.on("activate", function () {
    // On macOS it's common to re-create a window in the app when the
    // dock icon is clicked and there are no other windows open.
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

// Quit when all windows are closed, except on macOS.
// There, it's common for applications and their menu bar to stay active until
// the user quits  explicitly with Cmd + Q.
app.on("window-all-closed", function () {
  if (process.platform !== "darwin") {
    app.quit();
  }
});

// If your app has no need to navigate or only needs to navigate to known pages,
// it is a good idea to limit navigation outright to that known scope,
// disallowing any other kinds of navigation.
const allowedNavigationDestinations = "https://my-electron-app.com";
app.on("web-contents-created", (event, contents) => {
  contents.on("will-navigate", (event, navigationUrl) => {
    console.log("About to navigate!");
    const parsedUrl = new URL(navigationUrl);

    if (!allowedNavigationDestinations.includes(parsedUrl.origin)) {
      event.preventDefault();
    }
  });
});

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.
