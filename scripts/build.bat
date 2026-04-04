@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ======================================
echo IDCU Agent Build Script (MinGW 无VS版)
echo ======================================

:: 设置临时目录
set TMP=%~dp0tmp
set TEMP=%~dp0tmp
if not exist "%TMP%" mkdir "%TMP%" 2>nul

:: 强制清理旧缓存（解决生成器不匹配问题）
if exist build (
    echo 清理旧构建缓存...
    rd /s /q build
)
if exist out (
    rd /s /q out
)

:: 创建目录
mkdir out build 2>nul

:: 进入构建目录
cd build

:: 使用 MinGW 生成器
cmake -G "MinGW Makefiles" -DBUILD_TESTS=ON -DBUILD_SAMPLE_DLL=ON -DBUILD_DYNAMIC_LOADER_EXAMPLE=ON ..
if %errorlevel% neq 0 (
    echo.
    echo CMake 生成失败！
    pause
    exit /b 1
)

:: 编译
mingw32-make -j%NUMBER_OF_PROCESSORS%
if %errorlevel% neq 0 (
    echo.
    echo 编译失败！
    pause
    exit /b 1
)

:: 复制配置文件（可执行文件已由CMake直接输出到out目录）
cd ..
copy /Y config\agent.cfg out\ >nul

echo.
echo ======================================
echo 编译成功！
echo 程序：out\idcu_agent.exe
echo ======================================
echo.
pause