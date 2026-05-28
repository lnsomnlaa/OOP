param(
    [Parameter(Mandatory = $true)][string]$DllPath,
    [Parameter(Mandatory = $true)][string]$KeyPath,
    [Parameter(Mandatory = $true)][long]$ExpiresUnix
)

$dllFull = (Resolve-Path -LiteralPath $DllPath).Path
$keyBytes = [IO.File]::ReadAllBytes((Resolve-Path -LiteralPath $KeyPath).Path)
$sha = (Get-FileHash -Algorithm SHA256 -LiteralPath $dllFull).Hash.ToLowerInvariant()
$msg = "$sha|$ExpiresUnix"
$hmac = New-Object System.Security.Cryptography.HMACSHA256(,$keyBytes)
$mac = [BitConverter]::ToString($hmac.ComputeHash([Text.Encoding]::ASCII.GetBytes($msg))).Replace("-", "").ToLowerInvariant()
$sigPath = $dllFull + ".sig"
@"
expires=$ExpiresUnix
sha256=$sha
hmac=$mac
"@ | Set-Content -LiteralPath $sigPath -Encoding ascii
Write-Host "Wrote $sigPath"
