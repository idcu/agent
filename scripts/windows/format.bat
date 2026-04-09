@echo off
echo Formatting C/C++ files...

for /r %%f in (*.c *.h) do (
    echo Formatting: %%f
    clang-format -i "%%f"
)

echo Formatting complete!
