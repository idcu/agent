@echo off
REM Auto format code (Windows version)

echo === Auto format code (clang-format) ===

REM Check if clang-format is installed
where clang-format >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: clang-format not found, please install it first
    exit /b 1
)

REM Define directories to format
set DIRS=libs modules app tests

REM Format all files
echo Formatting source files...

for %%d in (%DIRS%) do (
    if exist %%d (
        for /r %%d %%f in (*.c *.h) do (
            clang-format -i "%%f"
            echo Formatted: %%f
        )
    )
)

echo.
echo Formatting completed
