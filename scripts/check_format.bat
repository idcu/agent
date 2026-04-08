@echo off
REM Check code format (Windows version)

echo === Check code format (clang-format) ===

REM Check if clang-format is installed
where clang-format >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: clang-format not found, please install it first
    exit /b 1
)

REM Define directories to check
set DIRS=libs modules app tests

REM Run clang-format check
echo Checking source files...
set NEED_FORMAT=0

for %%d in (%DIRS%) do (
    if exist %%d (
        for /r %%d %%f in (*.c *.h) do (
            clang-format --dry-run --Werror "%%f" >nul 2>nul
            if errorlevel 1 (
                echo Needs formatting: %%f
                set /a NEED_FORMAT+=1
            )
        )
    )
)

if %NEED_FORMAT% EQU 0 (
    echo.
    echo All files are properly formatted
) else (
    echo.
    echo %NEED_FORMAT% files need formatting
    echo.
    echo Run the following command to auto-fix:
    echo   scripts\format.bat
    exit /b 1
)
