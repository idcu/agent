@echo off
setlocal enabledelayedexpansion

REM IDCU Agent 模块创建脚本 (Windows)
REM 用法: create-module.bat <模块名称> [目标目录] [依赖项...]

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..
set TEMPLATE_DIR=%PROJECT_ROOT%\templates\module

REM 检查参数
if "%~1"=="" (
    echo 用法: %0 ^<模块名称^> [目标目录] [依赖项...]
    echo.
    echo 示例:
    echo   %0 my_module
    echo   %0 my_module modules\business
    echo   %0 my_module modules\business idcu-common idcu-log
    echo.
    exit /b 1
)

set MODULE_NAME=%~1
shift

REM 将模块名转换为目录名（小写+下划线）
set MODULE_DIR=%MODULE_NAME%
set MODULE_DIR_UPPER=%MODULE_NAME%
REM 简单的大小写转换
for %%i in (a b c d e f g h i j k l m n o p q r s t u v w x y z) do (
    set MODULE_DIR=!MODULE_DIR:%%i=%%i!
    set MODULE_DIR_UPPER=!MODULE_DIR_UPPER:%%i=%%i!
)
REM 替换连字符为下划线
set MODULE_DIR=%MODULE_DIR:-=_%
set MODULE_DIR_UPPER=%MODULE_DIR_UPPER:-=_%

REM 转换为大写
for %%i in (A B C D E F G H I J K L M N O P Q R S T U V W X Y Z) do (
    set MODULE_DIR=!MODULE_DIR:%%i=%%i!
    set "chr=%%i"
    set "MODULE_DIR_UPPER=!MODULE_DIR_UPPER:%%i=%%i!"
)

REM 转换为大写
call :to_upper MODULE_DIR_UPPER "%MODULE_DIR%"

REM 目标目录
set TARGET_DIR=modules\business\%MODULE_DIR%
if not "%~1"=="" (
    echo %~1 | findstr /b "idcu-" >nul
    if errorlevel 1 (
        set TARGET_DIR=%~1\%MODULE_DIR%
        shift
    )
)

REM 依赖项
set DEPENDENCIES=
set DEPENDENCIES_CMAKE=
set DEPENDENCIES_JSON=
:loop_deps
if "%~1"=="" goto end_loop_deps
set DEPENDENCIES=%DEPENDENCIES% %~1
set DEPENDENCIES_CMAKE=%DEPENDENCIES_CMAKE%    %~1

set DEPENDENCIES_JSON=%DEPENDENCIES_JSON%    "%~1",
shift
goto loop_deps
:end_loop_deps

REM 移除最后一个逗号
if not "%DEPENDENCIES_JSON%"=="" (
    set DEPENDENCIES_JSON=%DEPENDENCIES_JSON:~0,-1%
)

set FULL_TARGET_DIR=%PROJECT_ROOT%\%TARGET_DIR%

echo 正在创建模块: %MODULE_NAME%
echo 目标目录: %FULL_TARGET_DIR%

REM 检查目录是否已存在
if exist "%FULL_TARGET_DIR%" (
    echo 错误: 目录已存在: %FULL_TARGET_DIR%
    exit /b 1
)

REM 创建目录结构
mkdir "%FULL_TARGET_DIR%\src" 2>nul
mkdir "%FULL_TARGET_DIR%\include\idcu\%MODULE_DIR%" 2>nul
mkdir "%FULL_TARGET_DIR%\tests" 2>nul

echo 生成文件...

REM 使用 PowerShell 进行模板替换
powershell -Command "& { . \"%SCRIPT_DIR%create-module-helper.ps1\"; Create-Module }"

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
echo   4. 在主 CMakeLists.txt 中添加模块目录（如需要）
echo.

exit /b 0

:to_upper
set "str=%~2"
set "result="
for /L %%i in (0,1,255) do (
    set "char=!str:~%%i,1!"
    if "!char!"=="" goto :eof
    for %%j in (a A b B c C d D e E f F g G h H i I j J k K l L m M n N o O p P q Q r R s S t T u U v V w W x X y Y z Z) do (
        if "!char!"=="%%j" set "char=%%j"
    )
    set "result=!result!!char!"
)
set "%~1=%result%"
goto :eof
