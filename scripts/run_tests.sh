#!/bin/bash
# IDCU Agent 测试运行脚本
# 运行所有核心库的单元测试

echo "========================================"
echo "  IDCU Agent 测试运行脚本"
echo "========================================"
echo ""

BUILD_DIR="build_test"
TEST_PASSED=0
TEST_FAILED=0

# 创建构建目录
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "[1/4] 配置 CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Debug
if [ $? -ne 0 ]; then
    echo "ERROR: CMake 配置失败！"
    cd ..
    exit 1
fi

echo ""
echo "[2/4] 编译项目..."
cmake --build . --config Debug
if [ $? -ne 0 ]; then
    echo "ERROR: 编译失败！"
    cd ..
    exit 1
fi

echo ""
echo "========================================"
echo "  运行测试"
echo "========================================"
echo ""

# 运行测试函数
run_test() {
    local test_path="$1"
    if [ -f "$test_path" ]; then
        chmod +x "$test_path"
        "$test_path"
        if [ $? -eq 0 ]; then
            TEST_PASSED=$((TEST_PASSED + 1))
        else
            TEST_FAILED=$((TEST_FAILED + 1))
        fi
    fi
}

# 测试 idcu-common
echo ""
echo "[测试 idcu-common]"
run_test "libs/idcu-common/tests/test_atomic"
run_test "libs/idcu-common/tests/test_lock"
run_test "libs/idcu-common/tests/test_vector"
run_test "libs/idcu-common/tests/test_hash_map"
run_test "libs/idcu-common/tests/test_error_code"
run_test "libs/idcu-common/tests/test_linked_list"
run_test "libs/idcu-common/tests/test_string_buf"

# 测试 idcu-log
echo ""
echo "[测试 idcu-log]"
run_test "libs/idcu-log/tests/test_log_init"
run_test "libs/idcu-log/tests/test_log_level"
run_test "libs/idcu-log/tests/test_log_format"
run_test "libs/idcu-log/tests/test_log_file"
run_test "libs/idcu-log/tests/test_log_comprehensive"

# 测试 idcu-config
echo ""
echo "[测试 idcu-config]"
run_test "libs/idcu-config/tests/test_config_basic"
run_test "libs/idcu-config/tests/test_config_comprehensive"

# 测试 idcu-network
echo ""
echo "[测试 idcu-network]"
run_test "libs/idcu-network/tests/test_network_init"
run_test "libs/idcu-network/tests/test_network_socket"
run_test "libs/idcu-network/tests/test_network_server"

# 测试 idcu-msgbus
echo ""
echo "[测试 idcu-msgbus]"
run_test "libs/idcu-msgbus/tests/test_msg_bus"

# 测试 idcu-coroutine
echo ""
echo "[测试 idcu-coroutine]"
run_test "libs/idcu-coroutine/tests/test_coroutine"

echo ""
echo "========================================"
echo "  测试总结"
echo "========================================"
echo "通过: $TEST_PASSED"
echo "失败: $TEST_FAILED"
echo "========================================"

cd ..

if [ $TEST_FAILED -gt 0 ]; then
    echo ""
    echo "有 $TEST_FAILED 个测试失败！"
    exit 1
else
    echo ""
    echo "所有测试通过！"
    exit 0
fi
