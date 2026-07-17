[CmdletBinding()]
param(
    [string]$Configuration = "Release",
    [string]$Runtime = "win-x64"
)

$ErrorActionPreference = "Stop"

$root = $PSScriptRoot
$project = Join-Path $root "ZenJiggle.csproj"
$distDir = Join-Path $root "dist"

Write-Host "== Removing previous build output (bin, obj, dist) ==" -ForegroundColor Cyan
foreach ($dir in @("bin", "obj", "dist")) {
    $path = Join-Path $root $dir
    if (Test-Path $path) {
        Remove-Item -Recurse -Force $path
    }
}

Write-Host "== Restoring dependencies (no cache) ==" -ForegroundColor Cyan
dotnet restore $project --force --no-cache
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "== Publishing $Configuration build for $Runtime ==" -ForegroundColor Cyan
dotnet publish $project `
    -c $Configuration `
    -r $Runtime `
    --self-contained false `
    -p:PublishSingleFile=true `
    -o $distDir
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host ""
Write-Host "Build succeeded: $distDir\ZenJiggle.exe" -ForegroundColor Green
