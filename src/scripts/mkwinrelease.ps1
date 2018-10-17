if (-not (Test-Path env:QT_STATIC)) { echo "QT_STATIC is not set. Please set it to the directory with the static build of QT"; exit; }
if (-not (Test-Path env:APP_VERSION)) { echo "APP_VERSION is not set. Please set it to the version you want to build, like 0.1.6"; exit; }

$target="zcash-qt-wallet-v$Env:APP_VERSION"

echo "Cleaning"
nmake clean *>$null
Remove-Item -Path debug -Recurse | Out-Null
Remove-Item -Path release -Recurse | Out-Null

# Run qmake
echo "Configuring"
& "$Env:QT_STATIC\bin\qmake.exe" zcash-qt-wallet.pro -spec win32-msvc "CONFIG+=release"

echo "Building"
nmake *>$null
# Make a dist directory in release
New-Item release/$target -itemtype directory | Out-Null
Move-Item release/zcash-qt-wallet.exe release/$target | Out-Null

echo "Copying"
& "$Env:QT_STATIC\bin\windeployqt.exe" release/$target/zcash-qt-wallet.exe *>$null
Copy-Item LICENSE release/$target | Out-Null
Copy-Item README.md release/$target | Out-Null

echo "Zipping"
Compress-Archive -LiteralPath release/$target -DestinationPath "release/$target.zip"

echo "Done"