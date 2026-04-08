@echo off
chcp 65001 >nul
echo ==========================================
echo   IDCU 一键构建系统 - 小白友好版
echo ==========================================
echo.

set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..
set BUILD_DIR=%PROJECT_DIR%\build

echo [1/4] 检查环境...
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ 错误：找不到 CMake！请先安装 CMake。
    echo    下载地址：https://cmake.org/download/
    pause
    exit /b 1
)
echo ✅ CMake 已安装

where gcc >nul 2>&1
if %errorlevel% neq 0 (
    where cl >nul 2>&1
    if %errorlevel% neq 0 (
        echo ❌ 错误：找不到编译器！请安装 MinGW-w64 或 Visual Studio。
        echo    MinGW-w64: https://www.mingw-w64.org/
        echo    Visual Studio: https://visualstudio.microsoft.com/
        pause
        exit /b 1
    )
    echo ✅ MSVC 编译器已找到
) else (
    echo ✅ GCC 编译器已找到
)

echo.
echo [2/4] 创建构建目录...
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)
cd /d "%BUILD_DIR%"
echo ✅ 构建目录已准备

echo.
echo [3/4] 配置 CMake 项目...
cmake .. -G "MinGW Makefiles" 2>nul
if %errorlevel% neq 0 (
    echo 尝试使用 Visual Studio...
    cmake .. -G "Visual Studio 16 2019" -A x64 2>nul
    if %errorlevel% neq 0 (
        cmake .. -G "Visual Studio 17 2022" -A x64 2>nul
        if %errorlevel% neq 0 (
            echo 尝试使用默认生成器...
            cmake ..
            if %errorlevel% neq 0 (
                echo ❌ CMake 配置失败！
                pause
                exit /b 1
            )
        )
    )
)
echo ✅ CMake 配置成功

echo.
echo [4/4] 编译项目...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo ❌ 编译失败！
    pause
    exit /b 1
)

echo.
echo ==========================================
echo   ✅ 构建成功！
echo ==========================================
echo.
echo 输出文件在：%BUILD_DIR%
echo.
pause
