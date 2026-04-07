@echo off
REM 自动格式化代码 (Windows 版本)

echo === 自动格式化代码 (clang-format) ===

REM 检查 clang-format 是否安装
where clang-format >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo 错误: 未找到 clang-format，请先安装
    exit /b 1
)

REM 定义要格式化的目录
set DIRS=libs modules app tests

REM 格式化所有文件
echo 正在格式化源文件...

for %%d in (%DIRS%) do (
    if exist %%d (
        for /r %%d %%f in (*.c *.h) do (
            clang-format -i "%%f"
            echo 已格式化: %%f
        )
    )
)

echo.
echo ✅ 格式化完成
