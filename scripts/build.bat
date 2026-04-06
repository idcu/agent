@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

REM ======================================
REM IDCU Agent 统一构建脚本 (Windows)
REM ======================================

REM 默认参数
set "BUILD_TYPE=Release"
set "COMPILER=auto"
set "WITH_BENCHMARK=ON"
set "WITH_TESTS=OFF"
set "CLEAN_BUILD=OFF"
set "VERBOSE=OFF"
set "HELP=0"

REM 获取脚本目录
for %%i in ("%~dp0..") do set "PROJECT_ROOT=%%~fi"
set "SCRIPT_DIR=%~dp0"

REM 颜色输出 (使用 ANSI 转义序列)
set "ESC="
for /F %%a in ('echo prompt $E ^| cmd') do set "ESC=%%a"
set "RED=%ESC%[31m"
set "GREEN=%ESC%[32m"
set "YELLOW=%ESC%[33m"
set "BLUE=%ESC%[34m"
set "NC=%ESC%[0m"

REM 显示帮助信息
:show_help
echo 用法: %0 [选项]
echo.
echo 选项:
echo   -h, --help                显示此帮助信息
echo   -t, --build-type TYPE     设置构建类型 (Debug/Release/RelWithDebInfo) [默认: Release]
echo   -c, --compiler COMPILER   指定编译器 (msvc/mingw) [默认: auto]
echo   -b, --with-benchmark      包含基准测试 [默认: ON]
echo   --no-benchmark            不包含基准测试
echo   -T, --with-tests          包含测试 [默认: OFF]
echo   -C, --clean               清理旧构建后重新构建
echo   -v, --verbose             显示详细输出
echo.
echo 示例:
echo   %0                                      # 自动检测编译器，Release 构建
echo   %0 -t Debug -C                          # Debug 构建，清理旧缓存
echo   %0 -t Release -c msvc                   # Release 构建，使用 MSVC
echo   %0 -t Debug -c mingw --with-tests       # Debug 构建，使用 MinGW，包含测试
echo.
echo 平台兼容性:
echo   - Windows: MSVC 2017+ / MinGW-w64 7.0+
echo   - MSVC 需要 Visual Studio 2017 或更高版本
echo   - MinGW 需要 MinGW-w64 (推荐 MSYS2)
echo.
exit /b 0

REM 解析命令行参数
:parse_args
if "%~1"=="" goto end_parse_args

if /i "%~1"=="-h" goto show_help
if /i "%~1"=="--help" goto show_help

if /i "%~1"=="-t" (
    set "BUILD_TYPE=%~2"
    shift
    shift
    goto parse_args
)
if /i "%~1"=="--build-type" (
    set "BUILD_TYPE=%~2"
    shift
    shift
    goto parse_args
)

if /i "%~1"=="-c" (
    set "COMPILER=%~2"
    shift
    shift
    goto parse_args
)
if /i "%~1"=="--compiler" (
    set "COMPILER=%~2"
    shift
    shift
    goto parse_args
)

if /i "%~1"=="-b" (
    set "WITH_BENCHMARK=ON"
    shift
    goto parse_args
)
if /i "%~1"=="--with-benchmark" (
    set "WITH_BENCHMARK=ON"
    shift
    goto parse_args
)
if /i "%~1"=="--no-benchmark" (
    set "WITH_BENCHMARK=OFF"
    shift
    goto parse_args
)

if /i "%~1"=="-T" (
    set "WITH_TESTS=ON"
    shift
    goto parse_args
)
if /i "%~1"=="--with-tests" (
    set "WITH_TESTS=ON"
    shift
    goto parse_args
)

if /i "%~1"=="-C" (
    set "CLEAN_BUILD=ON"
    shift
    goto parse_args
)
if /i "%~1"=="--clean" (
    set "CLEAN_BUILD=ON"
    shift
    goto parse_args
)

if /i "%~1"=="-v" (
    set "VERBOSE=ON"
    shift
    goto parse_args
)
if /i "%~1"=="--verbose" (
    set "VERBOSE=ON"
    shift
    goto parse_args
)

echo %RED%错误: 未知参数 %~1%NC%
echo.
goto show_help

:end_parse_args

REM 转换构建类型为小写
set "BUILD_TYPE_LOWER=%BUILD_TYPE%"
for %%L in (a b c d e f g h i j k l m n o p q r s t u v w x y z) do (
    set "BUILD_TYPE_LOWER=!BUILD_TYPE_LOWER:%%L=%%L!"
)

set "BUILD_DIR=%PROJECT_ROOT%\build"

REM 检查依赖
:check_dependencies
echo %BLUE%检查构建依赖...%NC%
echo.

REM 检查 CMake
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo %RED%错误: 未找到 CMake，请先安装 CMake 3.14+%NC%
    echo 下载地址: https://cmake.org/download/
    pause
    exit /b 1
)

for /f "tokens=3" %%i in ('cmake --version ^| findstr /r "cmake version"') do set "CMAKE_VERSION=%%i"
echo %GREEN%✓ CMake 版本: %CMAKE_VERSION%%NC%

REM 检查最低版本要求
for /f "tokens=1,2 delims=." %%a in ("%CMAKE_VERSION%") do (
    set "CMAKE_MAJOR=%%a"
    set "CMAKE_MINOR=%%b"
)
if !CMAKE_MAJOR! lss 3 (
    echo %RED%错误: CMake 版本过低，需要 3.14+，当前为 %CMAKE_VERSION%%NC%
    pause
    exit /b 1
)
if !CMAKE_MAJOR! equ 3 if !CMAKE_MINOR! lss 14 (
    echo %RED%错误: CMake 版本过低，需要 3.14+，当前为 %CMAKE_VERSION%%NC%
    pause
    exit /b 1
)

REM 检测或选择编译器
if /i "%COMPILER%"=="auto" (
    echo %BLUE%自动检测编译器...%NC%
    
    REM 首先尝试检测 MSVC
    call :check_msvc
    if %errorlevel% equ 0 (
        set "COMPILER=msvc"
        echo %GREEN%✓ 自动选择: MSVC%NC%
    ) else (
        REM 尝试检测 MinGW
        call :check_mingw
        if %errorlevel% equ 0 (
            set "COMPILER=mingw"
            echo %GREEN%✓ 自动选择: MinGW%NC%
        ) else (
            echo %RED%错误: 未找到 MSVC 或 MinGW 编译器%NC%
            echo.
            echo MSVC: 请安装 Visual Studio 2017 或更高版本
            echo MinGW: 请安装 MinGW-w64 (推荐 MSYS2)
            pause
            exit /b 1
        )
    )
) else if /i "%COMPILER%"=="msvc" (
    call :check_msvc
    if %errorlevel% neq 0 (
        echo %RED%错误: 未找到 MSVC 编译器%NC%
        pause
        exit /b 1
    )
    echo %GREEN%✓ 选择: MSVC%NC%
) else if /i "%COMPILER%"=="mingw" (
    call :check_mingw
    if %errorlevel% neq 0 (
        echo %RED%错误: 未找到 MinGW 编译器%NC%
        pause
        exit /b 1
    )
    echo %GREEN%✓ 选择: MinGW%NC%
) else (
    echo %RED%错误: 不支持的编译器: %COMPILER%%NC%
    pause
    exit /b 1
)

echo.
goto :eof

:check_msvc
where cl >nul 2>nul
if %errorlevel% equ 0 (
    for /f "tokens=*" %%i in ('cl 2^>^&1 ^| findstr /r "Version"') do set "MSVC_INFO=%%i"
    echo %GREEN%✓ MSVC 可用: !MSVC_INFO!%NC%
    exit /b 0
)

REM 尝试查找 VS 安装并设置环境
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>nul
    where cl >nul 2>nul
    if %errorlevel% equ 0 (
        echo %GREEN%✓ 已初始化 VS 2022 环境%NC%
        exit /b 0
    )
)
if exist "C:\Program Files\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>nul
    where cl >nul 2>nul
    if %errorlevel% equ 0 (
        echo %GREEN%✓ 已初始化 VS 2019 环境%NC%
        exit /b 0
    )
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>nul
    where cl >nul 2>nul
    if %errorlevel% equ 0 (
        echo %GREEN%✓ 已初始化 VS 2017 环境%NC%
        exit /b 0
    )
)

exit /b 1

:check_mingw
where gcc >nul 2>nul
if %errorlevel% neq 0 (
    exit /b 1
)

for /f "tokens=*" %%i in ('gcc --version ^| findstr /r "gcc"') do set "GCC_INFO=%%i"
echo %GREEN%✓ MinGW 可用: !GCC_INFO!%NC%

where mingw32-make >nul 2>nul
if %errorlevel% neq 0 (
    echo %YELLOW%警告: 未找到 mingw32-make，尝试使用 make%NC%
    where make >nul 2>nul
    if %errorlevel% neq 0 (
        echo %RED%错误: 未找到 make 或 mingw32-make%NC%
        exit /b 1
    )
) else (
    echo %GREEN%✓ mingw32-make 可用%NC%
)

exit /b 0

REM 显示构建信息
:show_build_info
echo %YELLOW%========================================%NC%
echo %YELLOW%  IDCU Agent 构建配置%NC%
echo %YELLOW%========================================%NC%
echo   项目根目录:    %PROJECT_ROOT%
echo   构建目录:      %BUILD_DIR%
echo   编译器:        %COMPILER%
echo   构建类型:      %BUILD_TYPE%
echo   包含基准测试:  %WITH_BENCHMARK%
echo   包含测试:      %WITH_TESTS%
echo   清理构建:      %CLEAN_BUILD%
echo %YELLOW%========================================%NC%
echo.
goto :eof

REM 清理旧构建
:clean_build
if "%CLEAN_BUILD%"=="ON" (
    echo %BLUE%清理旧构建缓存...%NC%
    if exist "%BUILD_DIR%" (
        rd /s /q "%BUILD_DIR%"
        echo %GREEN%✓ 已清理构建目录%NC%
    )
    echo.
)
goto :eof

REM 执行构建
:build
echo %BLUE%开始构建...%NC%

REM 创建构建目录
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

REM 设置 CMake 参数
set "CMAKE_ARGS=-DBUILD_BENCHMARKS=%WITH_BENCHMARK% -DBUILD_TESTS=%WITH_TESTS%"
if "%VERBOSE%"=="ON" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_VERBOSE_MAKEFILE=ON"
)

if /i "%COMPILER%"=="msvc" (
    echo %BLUE%CMake 配置 (MSVC)...%NC%
    cmake -G "Visual Studio 17 2022" -A x64 %CMAKE_ARGS% "%PROJECT_ROOT%" 2>nul
    if %errorlevel% neq 0 (
        cmake -G "Visual Studio 16 2019" -A x64 %CMAKE_ARGS% "%PROJECT_ROOT%" 2>nul
        if %errorlevel% neq 0 (
            cmake -G "Visual Studio 15 2017 Win64" %CMAKE_ARGS% "%PROJECT_ROOT%"
            if %errorlevel% neq 0 (
                echo %RED%CMake 配置失败!%NC%
                pause
                exit /b 1
            )
        )
    )
    
    echo %BLUE%编译中...%NC%
    cmake --build . --config %BUILD_TYPE%
    if %errorlevel% neq 0 (
        echo %RED%编译失败!%NC%
        pause
        exit /b 1
    )
) else (
    echo %BLUE%CMake 配置 (MinGW)...%NC%
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% %CMAKE_ARGS% "%PROJECT_ROOT%"
    if %errorlevel% neq 0 (
        echo %RED%CMake 配置失败!%NC%
        pause
        exit /b 1
    )
    
    echo %BLUE%编译中...%NC%
    where mingw32-make >nul 2>nul
    if %errorlevel% equ 0 (
        mingw32-make -j%NUMBER_OF_PROCESSORS%
    ) else (
        make -j%NUMBER_OF_PROCESSORS%
    )
    
    if %errorlevel% neq 0 (
        echo %RED%编译失败!%NC%
        pause
        exit /b 1
    )
)

cd /d "%PROJECT_ROOT%"
goto :eof

REM 完成
:finish
echo.
echo %GREEN%========================================%NC%
echo %GREEN%  构建成功!%NC%
echo %GREEN%========================================%NC%

set "OUTPUT_DIR=%PROJECT_ROOT%\build\%BUILD_TYPE_LOWER%"
if exist "%PROJECT_ROOT%\build\output\%BUILD_TYPE_LOWER%" (
    set "OUTPUT_DIR=%PROJECT_ROOT%\build\output\%BUILD_TYPE_LOWER%"
)

echo   主程序: %OUTPUT_DIR%\bin\idcu_agent.exe
if exist "%OUTPUT_DIR%\bin\idcu_benchmark.exe" (
    echo   基准测试: %OUTPUT_DIR%\bin\idcu_benchmark.exe
)
echo   配置文件: %OUTPUT_DIR%\bin\config\
echo %GREEN%========================================%NC%
echo.
goto :eof

REM 主流程
echo %BLUE%========================================%NC%
echo %BLUE%  IDCU Agent 统一构建脚本%NC%
echo %BLUE%========================================%NC%
echo.

call :check_dependencies
call :show_build_info
call :clean_build
call :build
call :finish

pause
exit /b 0
