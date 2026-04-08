@echo off
REM Run clang-tidy static analysis (Windows version)

echo === Run clang-tidy static analysis ===

REM Check if clang-tidy is installed
where clang-tidy >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: clang-tidy not found, please install LLVM/Clang first
    echo Download: https://llvm.org/builds/
    exit /b 1
)

REM Use default build directory if not specified
if "%BUILD_DIR%"=="" (
    set BUILD_DIR=build
)

REM Check if compile database exists
if not exist "%BUILD_DIR%\compile_commands.json" (
    echo ERROR: compile_commands.json not found
    echo.
    echo Please run CMake with compile commands export:
    echo   cmake -B %BUILD_DIR% -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    exit /b 1
)

echo Using build directory: %BUILD_DIR%

REM Define directories to analyze
set DIRS=libs modules app

REM Find all C source files
echo Finding source files...
set FILES=
for %%d in (%DIRS%) do (
    if exist %%d (
        for /r %%d %%f in (*.c) do (
            if "%FILES%"=="" (
                set FILES=%%f
            ) else (
                set FILES=%FILES% %%f
            )
        )
    )
)

if "%FILES%"=="" (
    echo No source files found
    exit /b 1
)

REM Run clang-tidy
echo.
echo Running clang-tidy...
echo.
clang-tidy %FILES% -p %BUILD_DIR% --header-filter=".*"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Static analysis completed, no issues found
) else (
    echo.
    echo Static analysis found issues, please fix them
    exit /b 1
)
