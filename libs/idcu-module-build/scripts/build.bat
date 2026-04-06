@echo off
chcp 65001 >nul
REM IDCU Module Build - Windows Build Script
REM 通用跨平台构建脚本 - Windows 版本

setlocal enabledelayedexpansion

echo ========================================
echo IDCU Module Build - Windows Build Script
echo ========================================

set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..

if "%1"=="" (
    echo Usage: build.bat [clean^|debug^|release] [module-dir]
    echo.
    echo Examples:
    echo   build.bat                    - Build all in Release mode
    echo   build.bat debug              - Build all in Debug mode
    echo   build.bat clean              - Clean build directory
    echo   build.bat release examples\demo-shared - Build specific module
    exit /b 1
)

set BUILD_TYPE=Release
set MODULE_DIR=
set CLEAN_BUILD=0
set GENERATOR=
set ARCH=x64

:parse_args
if "%1"=="clean" (
    set CLEAN_BUILD=1
    shift
    goto parse_args
)
if "%1"=="debug" (
    set BUILD_TYPE=Debug
    shift
    goto parse_args
)
if "%1"=="release" (
    set BUILD_TYPE=Release
    shift
    goto parse_args
)
if not "%1"=="" (
    set MODULE_DIR=%1
    shift
    goto parse_args
)

if %CLEAN_BUILD% equ 1 (
    echo Cleaning build directories...
    if exist "%PROJECT_DIR%\build" rmdir /s /q "%PROJECT_DIR%\build"
    echo Clean complete!
    exit /b 0
)

:detect_vs
echo Detecting Visual Studio...

REM 检测可用的 Visual Studio 版本
set VS_VERSIONS=
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe" set VS_VERSIONS=!VS_VERSIONS! "Visual Studio 17 2022"
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\devenv.exe" set VS_VERSIONS=!VS_VERSIONS! "Visual Studio 17 2022"
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\devenv.exe" set VS_VERSIONS=!VS_VERSIONS! "Visual Studio 17 2022"

if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" set VS_VERSIONS=!VS_VERSIONS! "Visual Studio 16 2019"
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\IDE\devenv.exe" set VS_VERSIONS=!VS_VERSIONS! "Visual Studio 16 2019"
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\IDE\devenv.exe" set VS_VERSIONS=!VS_VERSIONS! "Visual Studio 16 2019"

if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\devenv.exe" set VS_VERSIONS=!VS_VERSIONS! "Visual Studio 15 2017"
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\Common7\IDE\devenv.exe" set VS_VERSIONS=!VS_VERSIONS! "Visual Studio 15 2017"
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\IDE\devenv.exe" set VS_VERSIONS=!VS_VERSIONS! "Visual Studio 15 2017"

REM 选择第一个可用的 Visual Studio 版本
for %%g in (%VS_VERSIONS%) do (
    if "!GENERATOR!"=="" set GENERATOR=%%g
)

REM 如果没有检测到 Visual Studio，尝试使用 MinGW Makefiles
if "!GENERATOR!"=="" (
    echo Visual Studio not found, trying MinGW...
    where gcc >nul 2>&1
    if !ERRORLEVEL! equ 0 (
        set GENERATOR="MinGW Makefiles"
        set ARCH=
    ) else (
        echo ERROR: No suitable generator found!
        echo Please install Visual Studio 2017+ or MinGW.
        exit /b 1
    )
)

echo Using generator: !GENERATOR!

set BUILD_DIR=%PROJECT_DIR%\build

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

if "%MODULE_DIR%"=="" (
    echo Building all modules...
    
    cd /d "%BUILD_DIR%"
    if "!ARCH!"=="" (
        cmake -G !GENERATOR! -DCMAKE_BUILD_TYPE=%BUILD_TYPE% "%PROJECT_DIR%"
    ) else (
        cmake -G !GENERATOR! -A !ARCH! -DCMAKE_BUILD_TYPE=%BUILD_TYPE% "%PROJECT_DIR%"
    )
    if errorlevel 1 (
        echo CMake configuration failed!
        exit /b 1
    )
    
    cmake --build . --config %BUILD_TYPE%
    if errorlevel 1 (
        echo Build failed!
        exit /b 1
    )
) else (
    echo Building module: %MODULE_DIR%
    
    set FULL_MODULE_DIR=%PROJECT_DIR%\%MODULE_DIR%
    if not exist "%FULL_MODULE_DIR%" (
        echo Module directory not found: %FULL_MODULE_DIR%
        exit /b 1
    )
    
    set MODULE_BUILD_DIR=%BUILD_DIR%\%MODULE_DIR%
    if not exist "%MODULE_BUILD_DIR%" mkdir "%MODULE_BUILD_DIR%"
    
    cd /d "%MODULE_BUILD_DIR%"
    if "!ARCH!"=="" (
        cmake -G !GENERATOR! -DCMAKE_BUILD_TYPE=%BUILD_TYPE% "%FULL_MODULE_DIR%"
    ) else (
        cmake -G !GENERATOR! -A !ARCH! -DCMAKE_BUILD_TYPE=%BUILD_TYPE% "%FULL_MODULE_DIR%"
    )
    if errorlevel 1 (
        echo CMake configuration failed for module!
        exit /b 1
    )
    
    cmake --build . --config %BUILD_TYPE%
    if errorlevel 1 (
        echo Build failed for module!
        exit /b 1
    )
)

echo.
echo ========================================
echo Build successful!
echo Build type: %BUILD_TYPE%
echo ========================================

endlocal
