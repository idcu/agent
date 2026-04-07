@echo off
REM 检查代码格式 (Windows 版本)

echo === 检查代码格式 (clang-format) ===

REM 检查 clang-format 是否安装
where clang-format >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo 错误: 未找到 clang-format，请先安装
    exit /b 1
)

REM 定义要检查的目录
set DIRS=libs modules app tests

REM 运行 clang-format 检查
echo 正在检查源文件...
set NEED_FORMAT=0

for %%d in (%DIRS%) do (
    if exist %%d (
        for /r %%d %%f in (*.c *.h) do (
            clang-format --dry-run --Werror "%%f" >nul 2>nul
            if errorlevel 1 (
                echo 需要格式化: %%f
                set /a NEED_FORMAT+=1
            )
        )
    )
)

if %NEED_FORMAT% EQU 0 (
    echo.
    echo ✅ 所有文件格式正确
) else (
    echo.
    echo ❌ %NEED_FORMAT% 个文件需要格式化
    echo.
    echo 运行以下命令自动修复:
    echo   scripts\format.bat
    exit /b 1
)
