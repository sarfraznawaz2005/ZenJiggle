# Installs ZenJiggle from dist/ZenJiggle.exe in this repo and starts it.
# Usage: irm https://raw.githubusercontent.com/sarfraznawaz2005/ZenJiggle/main/install.ps1 | iex

$ErrorActionPreference = 'Stop'

$url = 'https://raw.githubusercontent.com/sarfraznawaz2005/ZenJiggle/main/dist/ZenJiggle.exe'
$dir = Join-Path $env:LOCALAPPDATA 'ZenJiggle'
$exe = Join-Path $dir 'ZenJiggle.exe'

New-Item -ItemType Directory -Force -Path $dir | Out-Null

# Close a running copy so the file can be replaced.
Get-Process -Name ZenJiggle -ErrorAction SilentlyContinue | Stop-Process -Force

Write-Host "Downloading ZenJiggle to $exe ..."

# Older Windows PowerShell may not use TLS 1.2 by default, which GitHub requires.
[Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12

try {
    Invoke-WebRequest -Uri $url -OutFile $exe -UseBasicParsing
} catch {
    Write-Host 'First download failed. Trying another method ...'
    Start-BitsTransfer -Source $url -Destination $exe
}

Start-Process -FilePath $exe
Write-Host 'ZenJiggle is running in the system tray.'
