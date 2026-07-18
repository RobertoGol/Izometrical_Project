param(
    [string]$Configuration = "Debug",
    [string]$BuildDir = ""
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptDir "..\..")

if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $repoRoot "build-mapeditor"
}

# MSBuild treats environment variable names case-insensitively. Some shells can
# expose both PATH and Path, which makes CL.exe startup fail with MSB6001.
if (Test-Path Env:Path) {
    $env:PATH = $env:Path
    Remove-Item Env:Path -ErrorAction SilentlyContinue
}

$cmake = "cmake"
$bundledCmake = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if (Test-Path $bundledCmake) {
    $cmake = $bundledCmake
}

& $cmake -S $scriptDir -B $BuildDir
& $cmake --build $BuildDir --config $Configuration
