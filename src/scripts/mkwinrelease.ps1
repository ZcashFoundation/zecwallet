if (-not (Test-Path env:QT_DIR)) { echo "QT_DIR is not set. Please set it to the Qt directory from where you'd like to build. Usually, this is the installation directory of Qt."; exit; }
if (-not (Test-Path env:APP_VERSION)) { echo "APP_VERSION is not set. Please set it to the version you want to build, like 0.1.6"; exit; }

$target="zcash-qt-wallet-v$Env:APP_VERSION"

echo "Cleaning"
nmake clean *>$null
Remove-Item -Path debug -Recurse | Out-Null
Remove-Item -Path release -Recurse | Out-Null

# Run qmake
echo "Configuring"
& "$Env:QT_DIR\bin\qmake.exe" zcash-qt-wallet.pro -spec win32-msvc "CONFIG+=release"

echo "Building"
nmake *>$null
# Make a dist directory in release
New-Item release/$target -itemtype directory | Out-Null
Move-Item release/zcash-qt-wallet.exe release/$target | Out-Null

echo "Copying"
# No need for deploy, since we're using a static Qt5 library
& "$Env:QT_DIR\bin\windeployqt.exe" release/$target/zcash-qt-wallet.exe *>$null
Copy-Item LICENSE release/$target | Out-Null
Copy-Item README.md release/$target | Out-Null

echo "Zipping"
Compress-Archive -LiteralPath release/$target -DestinationPath "release/Windows-$target.zip"

echo "Package Contents"
[Reflection.Assembly]::LoadWithPartialName('System.IO.Compression.FileSystem')
foreach($sourceFile in (Get-ChildItem "release/Windows-$target.zip"))
{
    [IO.Compression.ZipFile]::OpenRead($sourceFile.FullName).Entries.FullName |
        %{ "$sourcefile`:$_" }
}

echo "Done"