# Builds the mod and packs binary\client into F:\mafia2-coop-client.zip for your brother.
# Run from anywhere:  powershell -ExecutionPolicy Bypass -File F:\mafia2-coop\make-zip.ps1
# Add -NoBuild to only re-zip.

param([switch]$NoBuild)

$repo   = $PSScriptRoot
$client = Join-Path $repo "binary\client"
$dll    = Join-Path $client "m2o-client.dll"
$zip    = "F:\mafia2-coop-client.zip"
$tmp    = Join-Path $env:TEMP "mafia2-coop-client"

if (Get-Process mafia2 -ErrorAction SilentlyContinue) {
    Write-Host "Mafia II is still running - close it first (it locks m2o-client.dll)." -ForegroundColor Red
    exit 1
}

if (-not $NoBuild) {
    $before = (Get-Item $dll -ErrorAction SilentlyContinue).LastWriteTime
    Push-Location $repo
    cmake --build build --config Release -- /m /v:minimal | Tee-Object -Variable out | Out-Null
    Pop-Location

    $errors = $out | Select-String -Pattern ": error " | Where-Object { $_ -notmatch "MSB3073" }
    if ($errors) {
        Write-Host "BUILD FAILED:" -ForegroundColor Red
        $errors | ForEach-Object { Write-Host $_.Line }
        exit 1
    }
    $after = (Get-Item $dll).LastWriteTime
    if ($after -eq $before) {
        Write-Host "Warning: m2o-client.dll did not change (nothing to rebuild?)." -ForegroundColor Yellow
    } else {
        Write-Host "Built m2o-client.dll at $after" -ForegroundColor Green
    }
}

Remove-Item $tmp -Recurse -Force -ErrorAction SilentlyContinue
robocopy $client $tmp /E /XF debug.log server.txt test.lua /XD exceptions | Out-Null
Compress-Archive -Path "$tmp\*" -DestinationPath $zip -Force
Remove-Item $tmp -Recurse -Force -ErrorAction SilentlyContinue

$z = Get-Item $zip
Write-Host ""
Write-Host ("Zip ready: {0}  ({1} MB, {2})" -f $z.FullName, [math]::Round($z.Length / 1MB, 1), $z.LastWriteTime) -ForegroundColor Green
Write-Host "DLL hash (your brother's should match after unzipping):"
Write-Host "  $((Get-FileHash $dll).Hash)"
