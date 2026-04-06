# 测试自动适配功能
cmake_minimum_required(VERSION 3.10)

project(test_auto_adapter)

# 包含模块构建系统
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(idcu_module_build)

message(STATUS "=== Testing Auto Adapter Functions ===")

# 测试 1: 自动检测当前项目类型 (应该是 idcu_agent)
message(STATUS "\n[Test 1] Auto detecting project type...")
auto_detect_project_type(
    PROJECT_DIR "${CMAKE_SOURCE_DIR}/../.."
    RESULT_VAR DETECTED_TYPE
    ADAPTER_RESULT_VAR DETECTED_ADAPTER
)
message(STATUS "Detected type: ${DETECTED_TYPE}")
message(STATUS "Detected adapter: ${DETECTED_ADAPTER}")

# 测试 2: 自动加载项目配置
message(STATUS "\n[Test 2] Auto loading project config...")
auto_load_project_config(
    PROJECT_DIR "${CMAKE_SOURCE_DIR}/../.."
    ADAPTER "${DETECTED_ADAPTER}"
)

# 测试 3: 自动应用构建策略
message(STATUS "\n[Test 3] Auto applying build strategy...")
auto_apply_build_strategy(
    PROJECT_DIR "${CMAKE_SOURCE_DIR}/../.."
)

message(STATUS "\n=== All Tests Completed ===")
