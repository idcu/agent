@echo off
chcp 65001 &gt;nul
title IDCU Agent - Format All Code

echo ========================================
echo   IDCU Agent - 格式化所有代码
echo ========================================
echo.

REM 设置 LLVM 路径
set LLVM_PATH=d:\os\run\llvm-mingw\llvm-mingw-20260324-ucrt-x86_64\bin
set PATH=%LLVM_PATH%;%PATH%

REM 检查 clang-format
where clang-format &gt;nul 2&gt;nul
if %errorlevel% neq 0 (
    echo ❌ 错误: 未找到 clang-format
    echo.
    pause
    exit /b 1
)

for /f "tokens=*" %%i in ('clang-format --version') do set CLANG_FORMAT_VERSION=%%i
echo ✓ %CLANG_FORMAT_VERSION%
echo.

REM 定义要格式化的目录
set DIRS=libs modules app tests

echo 正在格式化源代码...
echo.

set FORMAT_COUNT=0

for %%d in (%DIRS%) do (
    if exist %%d (
        for /r %%d %%f in (*.c *.h) do (
            clang-format -i "%%f"
            echo 已格式化: %%f
            set /a FORMAT_COUNT+=1
        )
    )
)

echo.
echo ========================================
echo   ✓ 格式化完成!
echo   共格式化 %FORMAT_COUNT% 个文件
echo ========================================
echo.
pause
