if (-not (Test-Path env:APP_VERSION)) { echo "APP_VERSION is not set. Please set it to the version you want to build, like 0.1.6"; exit; }
$target="zec-qt-wallet-v$Env:APP_VERSION"

echo "Building Installer"
Remove-Item -Path release/wininstaller -Recurse | Out-Null
New-Item release/wininstaller -itemtype directory | Out-Null

Copy-Item release/$target/zec-qt-wallet.exe release/wininstaller/
Copy-Item src/scripts/zec-qt-wallet.wxs release/wininstaller/

candle.exe release/wininstaller/zec-qt-wallet.wxs -o release/wininstaller/zec-qt-wallet.wixobj 
light.exe -ext WixUIExtension -cultures:en-us release/wininstaller/zec-qt-wallet.wixobj -out release/wininstaller/zec-qt-wallet.msi 

New-Item artifacts -itemtype directory -Force | Out-Null
Copy-Item release/wininstaller/zec-qt-wallet.msi ./artifacts/$target.msi