# Unified build script for windows, linux and mac builder. Run on a windows machine inside powershell.
param (
    [Parameter(Mandatory=$true)][string]$version,
    [Parameter(Mandatory=$true)][string]$prev,
    [Parameter(Mandatory=$true)][string]$server
)

Write-Host -NoNewline "Copying files....."
ssh $server "rm -rf /tmp/zqwbuild"
ssh $server "mkdir /tmp/zqwbuild"
scp -r * ${server}:/tmp/zqwbuild | Out-Null
ssh $server "dos2unix -q /tmp/zqwbuild/src/scripts/mkrelease.sh" | Out-Null
Write-Host "[OK]"

ssh $server "cd /tmp/zqwbuild && QT_STATIC=~/Qt/5.11.2/static/ ZCASH_DIR=~/github/zcash APP_VERSION=$version PREV_VERSION=$prev bash src/scripts/mkrelease.sh"
if (!$?) {
    Write-Output "Build failed"
    exit 1;
}

New-Item artifacts -itemtype directory -Force | Out-Null
scp ${server}:/tmp/zqwbuild/artifacts/* artifacts/ 