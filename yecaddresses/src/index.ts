import { encode, decode } from "./base58check.js";
import bech32 = require('bech32');
import $ from 'jquery';
import QRCode from 'qrcode';

const t1prefix = "1cb8";
const s1prefix = "1c28";
const t3prefix = "1cbd";
const s3prefix = "1c2c";
const zcprefix = "169a";
const ycprefix = "1636";

$("#convert_button").click( e => {
    var inp = $("#input_address").val().toString();

    if (inp.startsWith("t") || inp.startsWith("s") ||
        inp.startsWith("zc") || inp.startsWith("yc")) {
        doBase58(inp);
    } else if (inp.startsWith("zs") || inp.startsWith("ys")) {
        doBech32(inp);
    } else {
        showError("Unknown address format");
    }
});

function doBase58(inp: string) {
    var zcash_prefix, ycash_prefix;
    try {
        let d = decode(inp, "hex");
        let prefix = d.prefix.toString().toLowerCase();

        if (prefix == t1prefix || prefix == s1prefix) {
            zcash_prefix = t1prefix;
            ycash_prefix = s1prefix;
        } else if (prefix == t3prefix || prefix == s3prefix) {
            zcash_prefix = t3prefix;
            ycash_prefix = s3prefix;
        } else if (prefix == zcprefix || prefix == ycprefix) {
            zcash_prefix = zcprefix;
            ycash_prefix = ycprefix;
        }
        
        let ycashaddress = encode(d.data, ycash_prefix);
        $("#ycash_address").text(ycashaddress);
        QRCode.toCanvas(document.getElementById('ycash_address_canvas'), ycashaddress);

        let zcashaddress = encode(d.data, zcash_prefix);
        $("#zcash_address").text(zcashaddress);
        QRCode.toCanvas(document.getElementById('zcash_address_canvas'), zcashaddress);
    } catch (e) {
        showError(e);   
    }
}

function doBech32(inp: string) {
    try {
        let d = bech32.decode(inp);

        let ycashaddress = bech32.encode("ys", d.words);
        $("#ycash_address").text(ycashaddress);
        QRCode.toCanvas(document.getElementById('ycash_address_canvas'), ycashaddress);

        let zcashaddress = bech32.encode("zs", d.words);
        $("#zcash_address").text(zcashaddress);
        QRCode.toCanvas(document.getElementById('zcash_address_canvas'), zcashaddress);

    } catch (e) {
        showError(e);
    }
}

function showError(e: string) {
    $("#ycash_address").text(e);
    $("#zcash_address").text(e);
}
