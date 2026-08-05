# Build H3OS via WSL from PowerShell
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Write-Host "H3OS — building through WSL..." -ForegroundColor Cyan
wsl -e bash -lc "export PATH=`$HOME/.local/bin:`$PATH; cd '/mnt/f/my custom pc/app/H30S' 2>/dev/null || cd '$($root -replace '\\','/' -replace '^([A-Za-z]):','/mnt/$($root.Substring(0,1).ToLower())')'; pwd; bash tools/wsl-build.sh"
Write-Host "Done. Kernel at build/h3os.elf" -ForegroundColor Green
