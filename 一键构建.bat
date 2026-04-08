@echo off
chcp 65001 >nul
title IDCU Agent - 一键构建

echo ========================================
echo   IDCU Agent 一键构建工具
echo ========================================
echo.
echo 正在检查环境...
echo.

REM 检查 CMake
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo ❌ 错误: 未找到 CMake
    echo.
    echo 请先安装 CMake 3.14 或更高版本:
    echo 下载地址: https://cmake.org/download/
    echo.
    pause
    exit /b 1
)
echo ✓ CMake 已安装

REM 检查编译器 (MSVC 或 MinGW)
set COMPILER=
where cl >nul 2>nul
if %errorlevel% equ 0 (
    set COMPILER=msvc
    echo ✓ MSVC 编译器可用
) else (
    where gcc >nul 2>nul
    if %errorlevel% equ 0 (
        set COMPILER=mingw
        echo ✓ MinGW 编译器可用
    )
)

if "%COMPILER%"=="" (
    echo ❌ 错误: 未找到编译器
    echo.
    echo 请安装以下之一:
    echo   1. Visual Studio 2017+ (推荐)
    echo   2. MinGW-w64
    echo.
    pause
    exit /b 1
)

echo.
echo ========================================
echo 开始构建...
echo ========================================
echo.

REM 创建并进入构建目录
if not exist build mkdir build
cd build

REM 配置 CMake
echo 正在配置 CMake...
if "%COMPILER%"=="msvc" (
    cmake -G "Visual Studio 17 2022" -A x64 .. >nul 2>nul
    if %errorlevel% neq 0 (
        cmake -G "Visual Studio 16 2019" -A x64 .. >nul 2>nul
        if %errorlevel% neq 0 (
            cmake -G "Visual Studio 15 2017 Win64" ..
            if %errorlevel% neq 0 (
                echo ❌ CMake 配置失败
                pause
                exit /b 1
            )
        )
    )
) else (
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
    if %errorlevel% neq 0 (
        echo ❌ CMake 配置失败
        pause
        exit /b 1
    )
)

echo ✓ CMake 配置完成
echo.

REM 编译
echo 正在编译项目...
if "%COMPILER%"=="msvc" (
    cmake --build . --config Release
) else (
    where mingw32-make >nul 2>nul
    if %errorlevel% equ 0 (
        mingw32-make -j%NUMBER_OF_PROCESSORS%
    ) else (
        make -j%NUMBER_OF_PROCESSORS%
    )
)

if %errorlevel% neq 0 (
    echo.
    echo ❌ 编译失败，请查看上面的错误信息
    pause
    exit /b 1
)

cd ..

echo.
echo ========================================
echo   ✓ 构建成功!
echo ========================================
echo.
echo 输出位置:
echo   主程序: build\bin\idcu_agent.exe
echo   配置文件: build\bin\config\
echo.
echo 你可以运行 build\bin\idcu_agent.exe 来启动程序
echo.
pause
