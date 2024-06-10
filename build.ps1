param (
    [string]$vcpkgDir = $env:VCPKG_ROOT
)

if (-not $vcpkgDir) {
    Write-Output "Please provide vcpkg directory as argument or set VCPKG_ROOT environment variable."
    exit 1
}

$env:CMAKE_TOOLCHAIN_FILE = Join-Path $vcpkgDir "scripts\buildsystems\vcpkg.cmake"

if (-not (Test-Path build)) {
    New-Item -ItemType Directory -Path build
}

Set-Location build

if (-not (Test-Path CMakeCache.txt)) {
    cmake ..
    if ($LASTEXITCODE -ne 0) {
        Write-Output "CMake command failed."
        exit 1
    }
}

cmake --build .
Set-Location ..
dotnet build Viewer.Avalonia.Entry\Viewer.Avalonia.Entry.csproj