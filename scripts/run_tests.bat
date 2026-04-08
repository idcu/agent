@echo off
REM IDCU Agent 测试运行脚本
REM 运行所有核心库的单元测试

echo ========================================
echo   IDCU Agent 测试运行脚本
echo ========================================
echo.

set BUILD_DIR=build_test
set TEST_PASSED=0
set TEST_FAILED=0

REM 创建构建目录
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

echo [1/4] 配置 CMake...
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
if errorlevel 1 (
    echo ERROR: CMake 配置失败！
    cd ..
    exit /b 1
)

echo.
echo [2/4] 编译项目...
cmake --build . --config Debug
if errorlevel 1 (
    echo ERROR: 编译失败！
    cd ..
    exit /b 1
)

echo.
echo ========================================
echo   运行测试
echo ========================================
echo.

REM 测试 idcu-common
echo.
echo [测试 idcu-common]
if exist libs\idcu-common\tests\test_atomic.exe (
    libs\idcu-common\tests\test_atomic.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)
if exist libs\idcu-common\tests\test_lock.exe (
    libs\idcu-common\tests\test_lock.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)
if exist libs\idcu-common\tests\test_vector.exe (
    libs\idcu-common\tests\test_vector.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)
if exist libs\idcu-common\tests\test_hash_map.exe (
    libs\idcu-common\tests\test_hash_map.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)
if exist libs\idcu-common\tests\test_error_code.exe (
    libs\idcu-common\tests\test_error_code.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)
if exist libs\idcu-common\tests\test_linked_list.exe (
    libs\idcu-common\tests\test_linked_list.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)
if exist libs\idcu-common\tests\test_string_buf.exe (
    libs\idcu-common\tests\test_string_buf.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)

REM 测试 idcu-log
echo.
echo [测试 idcu-log]
if exist libs\idcu-log\tests\test_log_init.exe (
    libs\idcu-log\tests\test_log_init.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)
if exist libs\idcu-log\tests\test_log_level.exe (
    libs\idcu-log\tests\test_log_level.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)
if exist libs\idcu-log\tests\test_log_format.exe (
    libs\idcu-log\tests\test_log_format.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)
if exist libs\idcu-log\tests\test_log_file.exe (
    libs\idcu-log\tests\test_log_file.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)
if exist libs\idcu-log\tests\test_log_comprehensive.exe (
    libs\idcu-log\tests\test_log_comprehensive.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)

REM 测试 idcu-config
echo.
echo [测试 idcu-config]
if exist libs\idcu-config\tests\test_config_basic.exe (
    libs\idcu-config\tests\test_config_basic.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)
if exist libs\idcu-config\tests\test_config_comprehensive.exe (
    libs\idcu-config\tests\test_config_comprehensive.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)

REM 测试 idcu-network
echo.
echo [测试 idcu-network]
if exist libs\idcu-network\tests\test_network_init.exe (
    libs\idcu-network\tests\test_network_init.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)
if exist libs\idcu-network\tests\test_network_socket.exe (
    libs\idcu-network\tests\test_network_socket.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)
if exist libs\idcu-network\tests\test_network_server.exe (
    libs\idcu-network\tests\test_network_server.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)

REM 测试 idcu-msgbus
echo.
echo [测试 idcu-msgbus]
if exist libs\idcu-msgbus\tests\test_msg_bus.exe (
    libs\idcu-msgbus\tests\test_msg_bus.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)

REM 测试 idcu-coroutine
echo.
echo [测试 idcu-coroutine]
if exist libs\idcu-coroutine\tests\test_coroutine.exe (
    libs\idcu-coroutine\tests\test_coroutine.exe
    if errorlevel 1 (set /a TEST_FAILED+=1) else (set /a TEST_PASSED+=1)
)

echo.
echo ========================================
echo   测试总结
echo ========================================
echo 通过: %TEST_PASSED%
echo 失败: %TEST_FAILED%
echo ========================================

cd ..

if %TEST_FAILED% gtr 0 (
    echo.
    echo 有 %TEST_FAILED% 个测试失败！
    exit /b 1
) else (
    echo.
    echo 所有测试通过！
    exit /b 0
)
