import { RPCConfig } from "../components/AppState";

export {};

declare global {
  /**
   * We define all IPC APIs here to give devs auto-complete
   * use window.electron anywhere in app
   * Also note the capital "Window" here
   */
  interface Window {
    zecwallet: {
      onMenuItem: (
        name: string,
        callback: (arg1: any, arg2: any) => void
      ) => void;
      pathJoin: (dir: string, filename: string) => string;
      confDir: () => Promise<string>;
      rpcconfig: () => Promise<RPCConfig>;
      doRPC: (config: RPCConfig, method: string, params: any[]) => Promise<any>;
      getZecPrice: () => Promise<any>;
      showSaveDialog: (
        title: string,
        defaultPath: string,
        filters: any[],
        properties: string[]
      ) => Promise<any>;
      writeFile: (filename: string, content: string) => Promise<any>;
      readFile: (filename: string) => Promise<any>;
      copyToClipboard: (text: string) => void;
      openLink: (link: string) => void;
    };
    versions: {};
  }
}

// window.electron = window.electron || {};
