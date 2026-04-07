@echo off
setlocal enabledelayedexpansion

REM IDCU Agent 模块创建脚本 (Windows - 简化版)
REM 用法: create-module-simple.bat <模块名称> [目标目录]

if "%~1"=="" (
    echo 用法: %0 ^<模块名称^> [目标目录]
    echo.
    echo 示例:
    echo   %0 my_module
    echo   %0 my_module modules\business
    echo.
    exit /b 1
)

set MODULE_NAME=%~1
set MODULE_DIR=%MODULE_NAME:-=_%
set TARGET_DIR=modules\business\%MODULE_DIR%

if not "%~2"=="" (
    set TARGET_DIR=%~2\%MODULE_DIR%
)

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..
set TEMPLATE_DIR=%PROJECT_ROOT%\templates\module
set FULL_TARGET_DIR=%PROJECT_ROOT%\%TARGET_DIR%

echo 正在创建模块: %MODULE_NAME%
echo 目标目录: %FULL_TARGET_DIR%

if exist "%FULL_TARGET_DIR%" (
    echo 错误: 目录已存在: %FULL_TARGET_DIR%
    exit /b 1
)

mkdir "%FULL_TARGET_DIR%\src" 2>nul
mkdir "%FULL_TARGET_DIR%\include\idcu\%MODULE_DIR%" 2>nul
mkdir "%FULL_TARGET_DIR%\tests" 2>nul

echo 生成文件...

REM 使用 PowerShell 脚本来处理模板替换
powershell -ExecutionPolicy Bypass -File "%SCRIPT_DIR%create-module.ps1" "%MODULE_NAME%" "%MODULE_DIR%" "%TARGET_DIR%" "%FULL_TARGET_DIR%" "%TEMPLATE_DIR%"

if errorlevel 1 (
    echo 错误: 模块创建失败
    exit /b 1
)

echo 模块创建成功!
echo.
echo 已创建的文件:
echo   %TARGET_DIR%\CMakeLists.txt
echo   %TARGET_DIR%\README.md
echo   %TARGET_DIR%\module.json
echo   %TARGET_DIR%\src\%MODULE_DIR%.c
echo   %TARGET_DIR%\include\idcu\%MODULE_DIR%\%MODULE_DIR%.h
echo   %TARGET_DIR%\tests\CMakeLists.txt
echo   %TARGET_DIR%\tests\test_%MODULE_DIR%.c
echo.
echo 下一步:
echo   1. 编辑 %TARGET_DIR%\README.md 添加模块说明
echo   2. 在 %TARGET_DIR%\src\%MODULE_DIR%.c 中实现模块功能
echo   3. 在 %TARGET_DIR%\include\idcu\%MODULE_DIR%\%MODULE_DIR%.h 中添加公共 API
echo.
