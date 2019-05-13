# Unified build script for Windows, Linux and Mac builder. Run on a Windows machine inside powershell.
param (
    [Parameter(Mandatory=$true)][string]$version,
    [Parameter(Mandatory=$true)][string]$prev,
    [Parameter(Mandatory=$true)][string]$server
)

Write-Host "[Initializing]"
Remove-Item -Force -ErrorAction Ignore ./artifacts/linux-binaries-zec-qt-wallet-v$version.tar.gz
Remove-Item -Force -ErrorAction Ignore ./artifacts/linux-deb-zec-qt-wallet-v$version.deb
Remove-Item -Force -ErrorAction Ignore ./artifacts/Windows-binaries-zec-qt-wallet-v$version.zip
Remove-Item -Force -ErrorAction Ignore ./artifacts/Windows-installer-zec-qt-wallet-v$version.msi
Remove-Item -Force -ErrorAction Ignore ./artifacts/macOS-zec-qt-wallet-v$version.dmg

Remove-Item -Recurse -Force -ErrorAction Ignore ./bin
Remove-Item -Recurse -Force -ErrorAction Ignore ./debug
Remove-Item -Recurse -Force -ErrorAction Ignore ./release

# Create the version.h file and update README version number
Write-Output "#define APP_VERSION `"$version`"" > src/version.h
Get-Content README.md | Foreach-Object { $_ -replace "$prev", "$version" } | Out-File README-new.md
Move-Item -Force README-new.md README.md
Write-Host ""

Write-Host "[Building Linux + Windows]"
Write-Host -NoNewline "Copying files.........."
ssh $server "rm -rf /tmp/zqwbuild"
ssh $server "mkdir /tmp/zqwbuild"
scp -r src/ res/ ./silentdragon.pro ./application.qrc ./LICENSE ./README.md ${server}:/tmp/zqwbuild/ | Out-Null
ssh $server "dos2unix -q /tmp/zqwbuild/src/scripts/mkrelease.sh" | Out-Null
ssh $server "dos2unix -q /tmp/zqwbuild/src/version.h"
Write-Host "[OK]"

ssh $server "cd /tmp/zqwbuild && APP_VERSION=$version PREV_VERSION=$prev bash src/scripts/mkrelease.sh"
if (!$?) {
    Write-Output "[Error]"
    exit 1;
}

New-Item artifacts -itemtype directory -Force         | Out-Null
scp    ${server}:/tmp/zqwbuild/artifacts/* artifacts/ | Out-Null
scp -r ${server}:/tmp/zqwbuild/release .              | Out-Null

Write-Host -NoNewline "Building Installer....."
src/scripts/mkwininstaller.ps1 -version $version 2>&1 | Out-Null
if (!$?) {
    Write-Output "[Error]"
    exit 1;
}
Write-Host "[OK]"

# Finally, test to make sure all files exist
Write-Host -NoNewline "Checking Build........."
if (! (Test-Path ./artifacts/linux-binaries-zec-qt-wallet-v$version.tar.gz) -or
    ! (Test-Path ./artifacts/linux-deb-zec-qt-wallet-v$version.deb) -or
    ! (Test-Path ./artifacts/Windows-binaries-zec-qt-wallet-v$version.zip) -or
    ! (Test-Path ./artifacts/Windows-installer-zec-qt-wallet-v$version.msi) ) {
        Write-Host "[Error]"
        exit 1;
    }
Write-Host "[OK]"
