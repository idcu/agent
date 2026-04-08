@echo off
REM Build with AddressSanitizer (Windows version)

echo === Build with AddressSanitizer ===
echo.

REM Note about compiler requirement
echo Note: AddressSanitizer requires GCC or Clang compiler
echo.

REM Set build directory
set BUILD_DIR=build_asan

REM Clean old build directory
if exist %BUILD_DIR% (
    echo Cleaning old build directory...
    rmdir /s /q %BUILD_DIR%
)

REM Create build directory
mkdir %BUILD_DIR%
cd %BUILD_DIR%

REM Run CMake with AddressSanitizer enabled
echo Configuring CMake...
cmake .. -DENABLE_ADDRESS_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
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
echo When running programs, AddressSanitizer will automatically
echo detect memory issues. It's recommended to run tests to
echo check for memory leaks and other problems.
