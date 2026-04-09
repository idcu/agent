@echo off
echo Checking code format...

set HAS_ERRORS=0

for /r %%f in (*.c *.h) do (
    clang-format --dry-run --Werror "%%f" >nul 2>&1
    if errorlevel 1 (
        echo File needs formatting: %%f
        set HAS_ERRORS=1
    )
)

if %HAS_ERRORS% equ 0 (
    echo All files are properly formatted!
) else (
    echo Some files need formatting!
    exit /b 1
)
