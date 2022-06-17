import React from "react";

import { BrowserRouter as Router } from "react-router-dom";
import Routes from "../Routes";

const Root = () => (
  <Router>
    <Routes />
  </Router>
);

// const Root = () => {
//   console.log("Hello");
//   console.log(window.versions);
//   (async () => {
//     const config = await window.zecwallet.rpcconfig();
//     console.log(config);

//     const r = await window.zecwallet.doRPC(config, "getinfo", []);
//     console.log(r);

//     const p = await window.zecwallet.getZecPrice();
//     console.log(p);
//   })();

//   return <div>Hello</div>;
// };

export default Root;
