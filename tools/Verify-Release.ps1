param([Parameter(Mandatory=$true)][string]$Path)
$ErrorActionPreference='Stop'
$meta=Get-Content -LiteralPath (Join-Path $PSScriptRoot '..\release.json') -Raw | ConvertFrom-Json
$item=Get-Item -LiteralPath $Path
if($item.Length -ne $meta.portable_asset.bytes){throw 'Release ZIP size mismatch'}
$hash=(Get-FileHash -LiteralPath $item.FullName -Algorithm SHA256).Hash.ToLower()
if($hash -ne $meta.portable_asset.sha256){throw 'Release ZIP SHA-256 mismatch'}
Write-Output ('Verified '+$meta.portable_asset.name+' (v'+$meta.version+')')
