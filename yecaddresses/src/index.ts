import { MyLibrary } from './MyLibrary';
import { encode, decode } from "./base58check.js";
import bech32 = require('bech32');

console.log('See this in your browser console: Typescript Webpack Starter Launched');

let r = decode("t1hCr8orBbknRF5bV3WzshooiqhyqNEqse5", "hex");
console.log(encode(r.data, "1c28"));

let z = bech32.decode("zs1a8jz6ae3uyvjr08yh8y3va32r7dd6qxtdg3r43m5r5rhzrqm3fup7qeum5m08ctr22wc7qd03s9");
console.log(bech32.encode("ys", z.words));