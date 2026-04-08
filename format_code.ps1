# PowerShell script to format code using clang-format

$ErrorActionPreference = "Stop"

Write-Host "=== Auto format code (clang-format) ===" -ForegroundColor Cyan

# Add LLVM to PATH
$llvmPath = "d:\os\run\llvm-mingw\llvm-mingw-20260324-ucrt-x86_64\bin"
$env:PATH = "$llvmPath;$env:PATH"

# Check if clang-format is available
$clangFormatVersion = clang-format --version
Write-Host "clang-format version: $clangFormatVersion" -ForegroundColor Green

# Define directories to format
$dirs = @("libs", "modules", "app", "tests")

Write-Host "Formatting source files..." -ForegroundColor Yellow

$formattedCount = 0

foreach ($dir in $dirs) {
    if (Test-Path $dir) {
        Get-ChildItem -Path $dir -Recurse -Include "*.c", "*.h" | ForEach-Object {
            $filePath = $_.FullName
            clang-format -i $filePath
            Write-Host "Formatted: $filePath" -ForegroundColor Gray
            $formattedCount++
        }
    }
}

Write-Host ""
Write-Host "Formatting completed, $formattedCount files formatted" -ForegroundColor Green
