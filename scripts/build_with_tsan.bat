@echo off
REM Build with ThreadSanitizer (Windows version)

echo === Build with ThreadSanitizer ===
echo.

REM Note about compiler requirement
echo Note: ThreadSanitizer requires GCC or Clang compiler
echo.

REM Set build directory
set BUILD_DIR=build_tsan

REM Clean old build directory
if exist %BUILD_DIR% (
    echo Cleaning old build directory...
    rmdir /s /q %BUILD_DIR%
)

REM Create build directory
mkdir %BUILD_DIR%
cd %BUILD_DIR%

REM Run CMake with ThreadSanitizer enabled
echo Configuring CMake...
cmake .. -DENABLE_THREAD_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed
    cd ..
    exit /b 1
)

REM Build the project
echo.
echo Building project...
cmake --build . --config Debug
if %ERRORLEVEL% NEQ 0 (
    echo Build failed
    cd ..
    exit /b 1
)

cd ..
echo.
echo Build completed!
echo.
echo Executables are in: %BUILD_DIR%\Debug\bin\
echo.
echo When running programs, ThreadSanitizer will automatically
echo detect data race issues. It's recommended to run concurrent
echo related tests to check for data races.
