# Installs ZenJiggle from the latest GitHub release and starts it.
# Usage: irm https://raw.githubusercontent.com/sarfraznawaz2005/ZenJiggle/main/install.ps1 | iex

$ErrorActionPreference = 'Stop'

$url = 'https://github.com/sarfraznawaz2005/ZenJiggle/releases/latest/download/ZenJiggle.exe'
$dir = Join-Path $env:LOCALAPPDATA 'ZenJiggle'
$exe = Join-Path $dir 'ZenJiggle.exe'

New-Item -ItemType Directory -Force -Path $dir | Out-Null

# Close a running copy so the file can be replaced.
Get-Process -Name ZenJiggle -ErrorAction SilentlyContinue | Stop-Process -Force

Write-Host "Downloading ZenJiggle to $exe ..."
Invoke-WebRequest -Uri $url -OutFile $exe -UseBasicParsing

Start-Process -FilePath $exe
Write-Host 'ZenJiggle is running in the system tray.'
