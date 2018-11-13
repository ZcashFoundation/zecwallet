if (-not (Test-Path env:APP_VERSION)) { 
    echo "APP_VERSION is not set. Please set it to the version you want to build"; 
    echo '$Env:APP_VERSION = "0.3.0"'
    exit; 
}
$target="zec-qt-wallet-v$Env:APP_VERSION"

echo "Building Installer"
Remove-Item -Path release/wininstaller -Recurse | Out-Null
New-Item release/wininstaller -itemtype directory | Out-Null

Copy-Item release/$target/zec-qt-wallet.exe release/wininstaller/
Copy-Item release/$target/LICENSE release/wininstaller/
Copy-Item release/$target/README.md release/wininstaller/
Copy-Item release/$target/zcashd.exe release/wininstaller/
Copy-Item release/$target/zcash-cli.exe release/wininstaller/

cat src/scripts/zec-qt-wallet.wxs | % { $_ -replace "RELEASE_VERSION", "$Env:APP_VERSION" } > release/wininstaller/zec-qt-wallet.wxs

candle.exe release/wininstaller/zec-qt-wallet.wxs -o release/wininstaller/zec-qt-wallet.wixobj 
light.exe -ext WixUIExtension -cultures:en-us release/wininstaller/zec-qt-wallet.wixobj -out release/wininstaller/zec-qt-wallet.msi 

New-Item artifacts -itemtype directory -Force | Out-Null
Copy-Item release/wininstaller/zec-qt-wallet.msi ./artifacts/$target.msi