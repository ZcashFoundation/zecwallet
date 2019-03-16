param (
    [Parameter(Mandatory=$true)][string]$version
)

$target="zecwallet-v$version"

Remove-Item -Path release/wininstaller -Recurse -ErrorAction Ignore  | Out-Null
New-Item release/wininstaller -itemtype directory                    | Out-Null

Copy-Item release/$target/zecwallet.exe     release/wininstaller/
Copy-Item release/$target/LICENSE           release/wininstaller/
Copy-Item release/$target/README.md         release/wininstaller/
Copy-Item release/$target/zcashd.exe        release/wininstaller/
Copy-Item release/$target/zcash-cli.exe     release/wininstaller/

Get-Content src/scripts/zecwallet.wxs | ForEach-Object { $_ -replace "RELEASE_VERSION", "$version" } | Out-File -Encoding utf8 release/wininstaller/zecwallet.wxs

candle.exe release/wininstaller/zecwallet.wxs -o release/wininstaller/zecwallet.wixobj 
if (!$?) {
    exit 1;
}

light.exe -ext WixUIExtension -cultures:en-us release/wininstaller/zecwallet.wixobj -out release/wininstaller/zecwallet.msi 
if (!$?) {
    exit 1;
}

New-Item artifacts -itemtype directory -Force | Out-Null
Copy-Item release/wininstaller/zecwallet.msi ./artifacts/Windows-installer-$target.msi