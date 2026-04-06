# 多项目自动适配验证脚本
# 任务2.3: 多项目自动适配验证
cmake_minimum_required(VERSION 3.10)

message(STATUS "=========================================")
message(STATUS "  任务2.3: 多项目自动适配验证")
message(STATUS "=========================================")

# 设置模块路径
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(idcu_module_build)

# ==========================================
# 验证函数定义
# ==========================================

function(verify_project PROJECT_DIR PROJECT_NAME EXPECTED_TYPE)
    message(STATUS "\n-----------------------------------------")
    message(STATUS "  验证项目: ${PROJECT_NAME}")
    message(STATUS "-----------------------------------------")
    message(STATUS "项目目录: ${PROJECT_DIR}")

    # 自动检测项目类型
    auto_detect_project_type(
        PROJECT_DIR "${PROJECT_DIR}"
        RESULT_VAR DETECTED_TYPE
        ADAPTER_RESULT_VAR DETECTED_ADAPTER
    )

    message(STATUS "检测到的项目类型: ${DETECTED_TYPE}")
    message(STATUS "期望的项目类型: ${EXPECTED_TYPE}")

    if("${DETECTED_TYPE}" STREQUAL "${EXPECTED_TYPE}")
        message(STATUS "[OK] 项目类型检测正确!")
    else()
        message(WARNING "[FAIL] 项目类型检测错误!")
    endif()

    # 自动加载配置
    message(STATUS "\n自动加载项目配置...")
    auto_load_project_config(
        PROJECT_DIR "${PROJECT_DIR}"
        ADAPTER "${DETECTED_ADAPTER}"
    )

    # 自动应用构建策略
    message(STATUS "\n自动应用构建策略...")
    auto_apply_build_strategy(
        PROJECT_DIR "${PROJECT_DIR}"
    )

    message(STATUS "[OK] ${PROJECT_NAME} 验证完成!")

    # 返回结果
    if("${DETECTED_TYPE}" STREQUAL "${EXPECTED_TYPE}")
        set(VERIFY_RESULT TRUE PARENT_SCOPE)
    else()
        set(VERIFY_RESULT FALSE PARENT_SCOPE)
    endif()
endfunction()

# ==========================================
# 验证1: IDCU Agent 项目
# ==========================================

message(STATUS "\n\n=========================================")
message(STATUS "  验证1: IDCU Agent 项目")
message(STATUS "=========================================")

set(IDCU_AGENT_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")
verify_project("${IDCU_AGENT_DIR}" "IDCU Agent" "idcu_agent")
set(IDCU_AGENT_VERIFY_RESULT ${VERIFY_RESULT})

# ==========================================
# 验证2: 纯 C 动态库项目 (demo-shared)
# ==========================================

message(STATUS "\n\n=========================================")
message(STATUS "  验证2: 纯 C 动态库项目 (demo-shared)")
message(STATUS "=========================================")

set(DEMO_SHARED_DIR "${CMAKE_CURRENT_LIST_DIR}/examples/demo-shared")
verify_project("${DEMO_SHARED_DIR}" "demo-shared" "c_shared_library")
set(DEMO_SHARED_VERIFY_RESULT ${VERIFY_RESULT})

# ==========================================
# 验证3: 纯 C 静态库项目 (demo-static)
# ==========================================

message(STATUS "\n\n=========================================")
message(STATUS "  验证3: 纯 C 静态库项目 (demo-static)")
message(STATUS "=========================================")

set(DEMO_STATIC_DIR "${CMAKE_CURRENT_LIST_DIR}/examples/demo-static")
verify_project("${DEMO_STATIC_DIR}" "demo-static" "c_static_library")
set(DEMO_STATIC_VERIFY_RESULT ${VERIFY_RESULT})

# ==========================================
# 验证4: 自定义项目 (custom)
# ==========================================

message(STATUS "\n\n=========================================")
message(STATUS "  验证4: 自定义项目 (custom)")
message(STATUS "=========================================")

# 创建一个临时自定义项目进行测试
set(CUSTOM_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/examples/custom-test")

if(NOT EXISTS "${CUSTOM_PROJECT_DIR}")
    file(MAKE_DIRECTORY "${CUSTOM_PROJECT_DIR}")
    
    # 创建简单的 CMakeLists.txt
    file(WRITE "${CUSTOM_PROJECT_DIR}/CMakeLists.txt"
        "cmake_minimum_required(VERSION 3.10)\n"
        "project(custom-test)\n"
        "add_executable(custom-test main.c)\n"
    )
    
    # 创建一个简单的源文件
    file(WRITE "${CUSTOM_PROJECT_DIR}/main.c"
        "#include <stdio.h>\n"
        "int main() {\n"
        "    printf(\"Hello from custom project!\\n\");\n"
        "    return 0;\n"
        "}\n"
    )
endif()

verify_project("${CUSTOM_PROJECT_DIR}" "custom-test" "custom")
set(CUSTOM_VERIFY_RESULT ${VERIFY_RESULT})

# ==========================================
# 验证总结
# ==========================================

message(STATUS "\n\n=========================================")
message(STATUS "  验证总结")
message(STATUS "=========================================")

set(ALL_PASSED TRUE)

if(IDCU_AGENT_VERIFY_RESULT)
    message(STATUS "[OK] IDCU Agent 项目验证通过")
else()
    message(WARNING "[FAIL] IDCU Agent 项目验证失败")
    set(ALL_PASSED FALSE)
endif()

if(DEMO_SHARED_VERIFY_RESULT)
    message(STATUS "[OK] demo-shared 项目验证通过")
else()
    message(WARNING "[FAIL] demo-shared 项目验证失败")
    set(ALL_PASSED FALSE)
endif()

if(DEMO_STATIC_VERIFY_RESULT)
    message(STATUS "[OK] demo-static 项目验证通过")
else()
    message(WARNING "[FAIL] demo-static 项目验证失败")
    set(ALL_PASSED FALSE)
endif()

if(CUSTOM_VERIFY_RESULT)
    message(STATUS "[OK] custom 项目验证通过")
else()
    message(WARNING "[FAIL] custom 项目验证失败")
    set(ALL_PASSED FALSE)
endif()

message(STATUS "\n=========================================")
if(ALL_PASSED)
    message(STATUS "  所有验证通过! ✓")
else()
    message(WARNING "  部分验证失败! ✗")
endif()
message(STATUS "=========================================")
