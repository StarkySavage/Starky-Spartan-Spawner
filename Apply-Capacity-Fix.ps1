param([string]$GameFolder, [switch]$Restore)
$ErrorActionPreference = 'Stop'
if (-not $GameFolder) { $GameFolder = Read-Host 'Paste the Halo Campaign Evolved folder opened by Steam Browse local files' }
if (Get-Process | Where-Object { $_.ProcessName -match 'HaloCampaignEvolved|Meteorite' }) { throw 'Close Halo before patching.' }
$target = Join-Path $GameFolder 'Meteorite\Binaries\Win64\HaloSimulation_tag_release.dll'
$target = (Resolve-Path -LiteralPath $target).Path
$original = 'c8c144404adf61a9de821c996682a7e66abadd7e530397d3bbde31c123203bf7'
$patched = '2af831395762adf7aedf59677a655bbd8aa097ab80d037228ed26d84e3b0d8e6'
$expected = if ($Restore) { $patched } else { $original }
$desired = if ($Restore) { $original } else { $patched }
$bytes = [IO.File]::ReadAllBytes($target)
function Hash-Bytes([byte[]]$Value) {
    $algo = [Security.Cryptography.SHA256]::Create()
    try { return ([BitConverter]::ToString($algo.ComputeHash($Value))).Replace('-', '').ToLowerInvariant() }
    finally { $algo.Dispose() }
}
$current = Hash-Bytes $bytes
if ($current -eq $desired) { Write-Output 'Already in the requested state. No files changed.'; exit 0 }
if ($current -ne $expected) { throw 'Unsupported or modified simulation DLL. No changes made. This patch supports only Steam CU4 build 2026.08.11.1121610.2.' }
$edits = @(
    @{ Offset=270790; Before=[byte[]](0,0,0,58); After=[byte[]](0,0,0,88) },
    @{ Offset=37517; Before=[byte[]](0,0,0,58); After=[byte[]](0,0,0,88) },
    @{ Offset=273499; Before=[byte[]](0,0,0,58); After=[byte[]](0,0,0,88) },
    @{ Offset=11137840; Before=[byte[]](0,0,0,50,0,0,0,0); After=[byte[]](0,0,0,80,0,0,0,0) },
    @{ Offset=11137848; Before=[byte[]](0,0,0,50,0,0,0,0); After=[byte[]](0,0,0,80,0,0,0,0) }
)
foreach ($edit in $edits) {
    $from = if ($Restore) { $edit.After } else { $edit.Before }
    $to = if ($Restore) { $edit.Before } else { $edit.After }
    for ($i=0; $i -lt $from.Length; $i++) {
        if ($bytes[$edit.Offset+$i] -ne $from[$i]) { throw 'Unexpected patch bytes. No changes made.' }
        $bytes[$edit.Offset+$i] = $to[$i]
    }
}
if ((Hash-Bytes $bytes) -ne $desired) { throw 'Patched result did not match the expected hash. No changes made.' }
$backup = $target + '.SSS-' + $current.Substring(0,12) + '.bak'
if (Test-Path -LiteralPath $backup) {
    if ((Hash-Bytes ([IO.File]::ReadAllBytes($backup))) -ne $current) { throw 'Existing backup differs; nothing changed.' }
} else { Copy-Item -LiteralPath $target -Destination $backup }
if ((Hash-Bytes ([IO.File]::ReadAllBytes($backup))) -ne $current) { throw 'Backup verification failed.' }
$temp = $target + '.StarkySpawner-' + [Guid]::NewGuid().ToString('N') + '.tmp'
[IO.File]::WriteAllBytes($temp,$bytes)
if ((Hash-Bytes ([IO.File]::ReadAllBytes($target))) -ne $current) { throw 'Game file changed during preparation; original left untouched.' }
[IO.File]::Replace($temp,$target,[System.Management.Automation.Language.NullString]::Value)
if ((Hash-Bytes ([IO.File]::ReadAllBytes($target))) -ne $desired) { Copy-Item -LiteralPath $backup -Destination $target -Force; throw 'Verification failed; restored backup.' }
Write-Output 'Completed and verified. The previous DLL is backed up beside the game DLL.'



