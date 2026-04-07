# Module Build Core - 通用模块构建核心
# 基于 module.json 自动构建 C 语言模块
# 无项目特定依赖，纯通用核心

include(CMakeParseArguments)

# 包含依赖管理模块
include("${CMAKE_CURRENT_LIST_DIR}/module_dependency_manager.cmake")

# ==============================================================================
# JSON 安全访问辅助函数
# ==============================================================================

# 安全获取 JSON 成员 - 如果不存在返回空字符串
function(safe_json_get RESULT_VAR JSON_CONTENT)
    cmake_parse_arguments(
        SAFE_JSON
        ""
        ""
        "KEYS"
        ${ARGN}
    )

    set(CURRENT_JSON "${JSON_CONTENT}")
    set(FOUND TRUE)

    foreach(KEY ${SAFE_JSON_KEYS})
        # 先检查键是否存在
        string(JSON MEMBER_COUNT LENGTH "${CURRENT_JSON}")
        set(KEY_EXISTS FALSE)
        math(EXPR LAST_INDEX "${MEMBER_COUNT} - 1")
        foreach(I RANGE ${LAST_INDEX})
            string(JSON MEMBER_NAME MEMBER "${CURRENT_JSON}" ${I})
            if(MEMBER_NAME STREQUAL KEY)
                set(KEY_EXISTS TRUE)
                break()
            endif()
        endforeach()
        
        if(NOT KEY_EXISTS)
            set(FOUND FALSE)
            break()
        endif()
        
        string(JSON TEMP GET "${CURRENT_JSON}" "${KEY}")
        set(CURRENT_JSON "${TEMP}")
    endforeach()

    if(FOUND)
        set(${RESULT_VAR} "${CURRENT_JSON}" PARENT_SCOPE)
    else()
        set(${RESULT_VAR} "" PARENT_SCOPE)
    endif()
endfunction()

# ==============================================================================
# Auto Adapter Rule Engine - 自动适配规则引擎
# ==============================================================================

# 检测文件是否存在
function(_detect_file_exists PROJECT_DIR FILE_PATH RESULT_VAR)
    if(EXISTS "${PROJECT_DIR}/${FILE_PATH}")
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

# 检测目录是否存在
function(_detect_dir_exists PROJECT_DIR DIR_PATH RESULT_VAR)
    if(IS_DIRECTORY "${PROJECT_DIR}/${DIR_PATH}")
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

# 检测文件是否包含指定内容
function(_detect_file_contains PROJECT_DIR FILE_PATH CONTENT RESULT_VAR)
    set(FULL_PATH "${PROJECT_DIR}/${FILE_PATH}")
    if(NOT EXISTS "${FULL_PATH}")
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
        return()
    endif()
    
    file(READ "${FULL_PATH}" FILE_CONTENT)
    if("${FILE_CONTENT}" MATCHES "${CONTENT}")
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

# 解析 auto-adapter.yaml（使用 CMake 原生字符串处理）
function(_parse_auto_adapter_config CONFIG_DIR RESULT_VAR)
    set(AUTO_ADAPTER_PATH "${CONFIG_DIR}/auto-adapter.yaml")
    if(NOT EXISTS "${AUTO_ADAPTER_PATH}")
        message(WARNING "auto-adapter.yaml not found at: ${AUTO_ADAPTER_PATH}")
        set(${RESULT_VAR} "" PARENT_SCOPE)
        return()
    endif()
    
    file(READ "${AUTO_ADAPTER_PATH}" ADAPTER_CONTENT)
    
    # 初始化适配器列表
    set(ADAPTERS "")
    
    # 将内容按行分割
    string(REPLACE "\n" ";" LINES "${ADAPTER_CONTENT}")
    
    set(IN_ADAPTER OFF)
    set(IN_DETECT OFF)
    set(IN_CONFIG OFF)
    set(CURRENT_ADAPTER_NAME "")
    set(CURRENT_ADAPTER_PRIORITY "0")
    set(CURRENT_DETECT_RULES "")
    set(CURRENT_CONFIG_TYPE "")
    set(CURRENT_CONFIG_SDK_PATH "")
    set(CURRENT_CONFIG_OUTPUT_DIR "")
    set(CURRENT_CONFIG_C_STANDARD "99")
    set(CURRENT_CONFIG_NAMESPACE "")
    
    foreach(LINE ${LINES})
        # 去除行首尾空白
        string(STRIP "${LINE}" LINE_STRIPPED)
        
        # 跳过空行和注释
        if("${LINE_STRIPPED}" STREQUAL "" OR "${LINE_STRIPPED}" MATCHES "^#")
            continue()
        endif()
        
        # 检查是否开始新的 adapter
        if("${LINE_STRIPPED}" MATCHES "^- name:")
            # 如果之前有 adapter，先保存
            if(CURRENT_ADAPTER_NAME)
                set(RULES_STR "")
                foreach(RULE ${CURRENT_DETECT_RULES})
                    if(RULES_STR)
                        set(RULES_STR "${RULES_STR},${RULE}")
                    else()
                        set(RULES_STR "${RULE}")
                    endif()
                endforeach()
                set(ADAPTER "${CURRENT_ADAPTER_NAME}|${CURRENT_ADAPTER_PRIORITY}|${RULES_STR}|${CURRENT_CONFIG_TYPE}|${CURRENT_CONFIG_SDK_PATH}|${CURRENT_CONFIG_OUTPUT_DIR}|${CURRENT_CONFIG_C_STANDARD}|${CURRENT_CONFIG_NAMESPACE}")
                list(APPEND ADAPTERS "${ADAPTER}")
            endif()
            
            # 重置并开始新的 adapter
            string(REGEX REPLACE "^- name:[ ]*(.+)" "\\1" CURRENT_ADAPTER_NAME "${LINE_STRIPPED}")
            set(CURRENT_ADAPTER_PRIORITY "0")
            set(CURRENT_DETECT_RULES "")
            set(CURRENT_CONFIG_TYPE "")
            set(CURRENT_CONFIG_SDK_PATH "")
            set(CURRENT_CONFIG_OUTPUT_DIR "")
            set(CURRENT_CONFIG_C_STANDARD "99")
            set(CURRENT_CONFIG_NAMESPACE "")
            set(IN_ADAPTER ON)
            set(IN_DETECT OFF)
            set(IN_CONFIG OFF)
        
        # 检测 priority
        elseif("${LINE_STRIPPED}" MATCHES "^priority:" AND IN_ADAPTER)
            string(REGEX REPLACE "^priority:[ ]*(.+)" "\\1" CURRENT_ADAPTER_PRIORITY "${LINE_STRIPPED}")
        
        # 检测 detect 块开始
        elseif("${LINE_STRIPPED}" MATCHES "^detect:" AND IN_ADAPTER)
            set(IN_DETECT ON)
            set(IN_CONFIG OFF)
        
        # 检测 config 块开始
        elseif("${LINE_STRIPPED}" MATCHES "^config:" AND IN_ADAPTER)
            set(IN_DETECT OFF)
            set(IN_CONFIG ON)
        
        # 处理 detect 规则
        elseif(IN_DETECT AND "${LINE_STRIPPED}" MATCHES "^-")
            string(REGEX REPLACE "^-[ ]*(.+)" "\\1" RULE_CONTENT "${LINE_STRIPPED}")
            
            # 解析规则类型
            if("${RULE_CONTENT}" MATCHES "^exists:[ ]*(.+)")
                string(REGEX REPLACE "^exists:[ ]*(.+)" "\\1" PATH_VALUE "${RULE_CONTENT}")
                # 去掉引号
                string(REGEX REPLACE "^\"(.+)\"$" "\\1" PATH_VALUE "${PATH_VALUE}")
                string(REGEX REPLACE "^'(.+)'$" "\\1" PATH_VALUE "${PATH_VALUE}")
                list(APPEND CURRENT_DETECT_RULES "exists:${PATH_VALUE}")
            elseif("${RULE_CONTENT}" MATCHES "^not_exists:[ ]*(.+)")
                string(REGEX REPLACE "^not_exists:[ ]*(.+)" "\\1" PATH_VALUE "${RULE_CONTENT}")
                string(REGEX REPLACE "^\"(.+)\"$" "\\1" PATH_VALUE "${PATH_VALUE}")
                string(REGEX REPLACE "^'(.+)'$" "\\1" PATH_VALUE "${PATH_VALUE}")
                list(APPEND CURRENT_DETECT_RULES "not_exists:${PATH_VALUE}")
            elseif("${RULE_CONTENT}" MATCHES "^file_contains:[ ]*(.+)")
                string(REGEX REPLACE "^file_contains:[ ]*(.+)" "\\1" FC_CONTENT "${RULE_CONTENT}")
                # 解析文件路径和内容 - 简化处理，假设格式为 "file_contains: \"file.txt\" \"content\""
                # 先尝试提取引号内的内容
                if("${FC_CONTENT}" MATCHES "^\"([^\"]+)\"[ ]+\"([^\"]+)\"")
                    set(FILE_PATH "${CMAKE_MATCH_1}")
                    set(CONTENT "${CMAKE_MATCH_2}")
                    list(APPEND CURRENT_DETECT_RULES "file_contains:${FILE_PATH}:${CONTENT}")
                elseif("${FC_CONTENT}" MATCHES "^([^ ]+)[ ]+(.+)")
                    # 无引号格式
                    set(FILE_PATH "${CMAKE_MATCH_1}")
                    set(CONTENT "${CMAKE_MATCH_2}")
                    list(APPEND CURRENT_DETECT_RULES "file_contains:${FILE_PATH}:${CONTENT}")
                endif()
            endif()
        
        # 处理 config 项
        elseif(IN_CONFIG AND "${LINE_STRIPPED}" MATCHES "^[a-z_]+:")
            string(REGEX REPLACE "^([a-z_]+):[ ]*(.+)" "\\1" CONFIG_KEY "${LINE_STRIPPED}")
            string(REGEX REPLACE "^([a-z_]+):[ ]*(.+)" "\\2" CONFIG_VALUE "${LINE_STRIPPED}")
            # 去掉引号
            string(REGEX REPLACE "^\"(.+)\"$" "\\1" CONFIG_VALUE "${CONFIG_VALUE}")
            string(REGEX REPLACE "^'(.+)'$" "\\1" CONFIG_VALUE "${CONFIG_VALUE}")
            
            if("${CONFIG_KEY}" STREQUAL "type")
                set(CURRENT_CONFIG_TYPE "${CONFIG_VALUE}")
            elseif("${CONFIG_KEY}" STREQUAL "sdk_path")
                set(CURRENT_CONFIG_SDK_PATH "${CONFIG_VALUE}")
            elseif("${CONFIG_KEY}" STREQUAL "output_dir")
                set(CURRENT_CONFIG_OUTPUT_DIR "${CONFIG_VALUE}")
            elseif("${CONFIG_KEY}" STREQUAL "c_standard")
                set(CURRENT_CONFIG_C_STANDARD "${CONFIG_VALUE}")
            elseif("${CONFIG_KEY}" STREQUAL "namespace")
                set(CURRENT_CONFIG_NAMESPACE "${CONFIG_VALUE}")
            endif()
        endif()
    endforeach()
    
    # 保存最后一个 adapter
    if(CURRENT_ADAPTER_NAME)
        set(RULES_STR "")
        foreach(RULE ${CURRENT_DETECT_RULES})
            if(RULES_STR)
                set(RULES_STR "${RULES_STR},${RULE}")
            else()
                set(RULES_STR "${RULE}")
            endif()
        endforeach()
        set(ADAPTER "${CURRENT_ADAPTER_NAME}|${CURRENT_ADAPTER_PRIORITY}|${RULES_STR}|${CURRENT_CONFIG_TYPE}|${CURRENT_CONFIG_SDK_PATH}|${CURRENT_CONFIG_OUTPUT_DIR}|${CURRENT_CONFIG_C_STANDARD}|${CURRENT_CONFIG_NAMESPACE}")
        list(APPEND ADAPTERS "${ADAPTER}")
    endif()
    
    list(LENGTH ADAPTERS ADAPTER_COUNT)
    message(STATUS "Loaded ${ADAPTER_COUNT} adapters from configuration")
    set(${RESULT_VAR} "${ADAPTERS}" PARENT_SCOPE)
endfunction()

# 检查单个检测规则
function(_check_detect_rule PROJECT_DIR RULE RESULT_VAR)
    # 解析规则
    if("${RULE}" MATCHES "^exists:(.+)$")
        set(PATH "${CMAKE_MATCH_1}")
        _detect_file_exists("${PROJECT_DIR}" "${PATH}" RULE_PASSED)
        if(NOT RULE_PASSED)
            _detect_dir_exists("${PROJECT_DIR}" "${PATH}" RULE_PASSED)
        endif()
    elseif("${RULE}" MATCHES "^not_exists:(.+)$")
        set(PATH "${CMAKE_MATCH_1}")
        _detect_file_exists("${PROJECT_DIR}" "${PATH}" FILE_EXISTS)
        _detect_dir_exists("${PROJECT_DIR}" "${PATH}" DIR_EXISTS)
        if(NOT FILE_EXISTS AND NOT DIR_EXISTS)
            set(RULE_PASSED TRUE)
        else()
            set(RULE_PASSED FALSE)
        endif()
    elseif("${RULE}" MATCHES "^file_contains:([^:]+):(.+)$")
        set(FILE_PATH "${CMAKE_MATCH_1}")
        set(CONTENT "${CMAKE_MATCH_2}")
        _detect_file_contains("${PROJECT_DIR}" "${FILE_PATH}" "${CONTENT}" RULE_PASSED)
    else()
        message(WARNING "Unknown detect rule: ${RULE}")
        set(RULE_PASSED FALSE)
    endif()
    
    set(${RESULT_VAR} ${RULE_PASSED} PARENT_SCOPE)
endfunction()

# 检查 adapter 是否匹配
function(_check_adapter_match PROJECT_DIR ADAPTER RESULT_VAR)
    # 解析 adapter
    string(REPLACE "|" ";" ADAPTER_PARTS "${ADAPTER}")
    list(GET ADAPTER_PARTS 0 ADAPTER_NAME)
    list(GET ADAPTER_PARTS 1 ADAPTER_PRIORITY)
    list(GET ADAPTER_PARTS 2 ADAPTER_RULES_STR)
    string(REPLACE ";" "," ADAPTER_RULES_STR "${ADAPTER_RULES_STR}")
    string(REPLACE "," ";" ADAPTER_RULES "${ADAPTER_RULES_STR}")
    
    # 检查所有规则
    set(ALL_RULES_PASSED TRUE)
    foreach(RULE ${ADAPTER_RULES})
        if("${RULE}" STREQUAL "")
            continue()
        endif()
        _check_detect_rule("${PROJECT_DIR}" "${RULE}" RULE_PASSED)
        if(NOT RULE_PASSED)
            set(ALL_RULES_PASSED FALSE)
            break()
        endif()
    endforeach()
    
    set(${RESULT_VAR} ${ALL_RULES_PASSED} PARENT_SCOPE)
endfunction()

# 自动检测项目类型（基于规则）
function(auto_detect_project_type)
    cmake_parse_arguments(
        AUTO_DETECT
        ""
        "PROJECT_DIR;RESULT_VAR;ADAPTER_RESULT_VAR"
        ""
        ${ARGN}
    )
    
    if(NOT AUTO_DETECT_PROJECT_DIR)
        set(AUTO_DETECT_PROJECT_DIR "${CMAKE_SOURCE_DIR}")
    endif()
    
    message(STATUS "Auto-detecting project type for: ${AUTO_DETECT_PROJECT_DIR}")
    
    # 解析适配器配置
    _parse_auto_adapter_config("${MODULE_BUILD_CONFIG_DIR}" ADAPTERS)
    
    if(NOT ADAPTERS)
        message(STATUS "No adapters found, using default: custom")
        if(AUTO_DETECT_RESULT_VAR)
            set(${AUTO_DETECT_RESULT_VAR} "custom" PARENT_SCOPE)
        endif()
        if(AUTO_DETECT_ADAPTER_RESULT_VAR)
            set(${AUTO_DETECT_ADAPTER_RESULT_VAR} "" PARENT_SCOPE)
        endif()
        return()
    endif()
    
    # 按优先级排序适配器
    set(SORTED_ADAPTERS "")
    foreach(ADAPTER ${ADAPTERS})
        string(REPLACE "|" ";" ADAPTER_PARTS "${ADAPTER}")
        list(LENGTH ADAPTER_PARTS ADAPTER_PARTS_LENGTH)
        if(ADAPTER_PARTS_LENGTH LESS 2)
            message(WARNING "Invalid adapter format, skipping: ${ADAPTER}")
            continue()
        endif()
        list(GET ADAPTER_PARTS 1 ADAPTER_PRIORITY)
        list(APPEND SORTED_ADAPTERS "${ADAPTER_PRIORITY}|${ADAPTER}")
    endforeach()
    
    # 排序
    list(SORT SORTED_ADAPTERS)
    list(REVERSE SORTED_ADAPTERS)
    
    # 检查每个适配器
    set(MATCHED_ADAPTER "")
    set(MATCHED_ADAPTER_NAME "")
    foreach(PRIORITY_ADAPTER ${SORTED_ADAPTERS})
        # 解析出原始适配器（去掉优先级前缀）
        string(FIND "${PRIORITY_ADAPTER}" "|" PIPE_POS)
        if(PIPE_POS EQUAL -1)
            continue()
        endif()
        math(EXPR ADAPTER_START "${PIPE_POS} + 1")
        string(SUBSTRING "${PRIORITY_ADAPTER}" ${ADAPTER_START} -1 ADAPTER)
        
        _check_adapter_match("${AUTO_DETECT_PROJECT_DIR}" "${ADAPTER}" IS_MATCH)
        if(IS_MATCH)
            set(MATCHED_ADAPTER "${ADAPTER}")
            string(REPLACE "|" ";" ADAPTER_PARTS "${ADAPTER}")
            list(GET ADAPTER_PARTS 0 ADAPTER_NAME)
            set(MATCHED_ADAPTER_NAME "${ADAPTER_NAME}")
            message(STATUS "Matched adapter: ${ADAPTER_NAME}")
            break()
        endif()
    endforeach()
    
    if(NOT MATCHED_ADAPTER)
        message(STATUS "No adapter matched, using default: custom")
        set(MATCHED_ADAPTER_NAME "custom")
    endif()
    
    # 设置结果
    if(AUTO_DETECT_RESULT_VAR)
        set(${AUTO_DETECT_RESULT_VAR} "${MATCHED_ADAPTER_NAME}" PARENT_SCOPE)
    endif()
    if(AUTO_DETECT_ADAPTER_RESULT_VAR)
        set(${AUTO_DETECT_ADAPTER_RESULT_VAR} "${MATCHED_ADAPTER}" PARENT_SCOPE)
    endif()
    
    set(MODULE_BUILD_DETECTED_PROJECT_TYPE "${MATCHED_ADAPTER_NAME}" PARENT_SCOPE)
    message(STATUS "Detected project type: ${MATCHED_ADAPTER_NAME}")
endfunction()

# 自动加载项目配置
function(auto_load_project_config)
    cmake_parse_arguments(
        AUTO_LOAD
        ""
        "PROJECT_DIR;ADAPTER"
        ""
        ${ARGN}
    )
    
    if(NOT AUTO_LOAD_PROJECT_DIR)
        set(AUTO_LOAD_PROJECT_DIR "${CMAKE_SOURCE_DIR}")
    endif()
    
    # 如果没有提供 adapter，先检测
    if(NOT AUTO_LOAD_ADAPTER)
        auto_detect_project_type(
            PROJECT_DIR "${AUTO_LOAD_PROJECT_DIR}"
            ADAPTER_RESULT_VAR AUTO_LOAD_ADAPTER
        )
    endif()
    
    if(NOT AUTO_LOAD_ADAPTER)
        message(STATUS "No adapter to load config from")
        return()
    endif()
    
    # 解析 adapter
    string(REPLACE "|" ";" ADAPTER_PARTS "${AUTO_LOAD_ADAPTER}")
    list(GET ADAPTER_PARTS 0 ADAPTER_NAME)
    list(GET ADAPTER_PARTS 3 CONFIG_TYPE)
    list(GET ADAPTER_PARTS 4 CONFIG_SDK_PATH)
    list(GET ADAPTER_PARTS 5 CONFIG_OUTPUT_DIR)
    list(GET ADAPTER_PARTS 6 CONFIG_C_STANDARD)
    list(GET ADAPTER_PARTS 7 CONFIG_NAMESPACE)
    
    message(STATUS "Loading config from adapter: ${ADAPTER_NAME}")
    
    # 设置配置变量
    if(CONFIG_TYPE)
        set(MODULE_BUILD_CONFIG_TYPE "${CONFIG_TYPE}" PARENT_SCOPE)
        message(STATUS "  type: ${CONFIG_TYPE}")
    endif()
    
    if(CONFIG_SDK_PATH)
        set(MODULE_BUILD_CONFIG_SDK_PATH "${CONFIG_SDK_PATH}" PARENT_SCOPE)
        message(STATUS "  sdk_path: ${CONFIG_SDK_PATH}")
    endif()
    
    if(CONFIG_OUTPUT_DIR)
        set(MODULE_BUILD_CONFIG_OUTPUT_DIR "${CONFIG_OUTPUT_DIR}" PARENT_SCOPE)
        message(STATUS "  output_dir: ${CONFIG_OUTPUT_DIR}")
    endif()
    
    if(CONFIG_C_STANDARD)
        set(MODULE_BUILD_CONFIG_C_STANDARD "${CONFIG_C_STANDARD}" PARENT_SCOPE)
        message(STATUS "  c_standard: ${CONFIG_C_STANDARD}")
    endif()
    
    if(CONFIG_NAMESPACE)
        set(MODULE_BUILD_CONFIG_NAMESPACE "${CONFIG_NAMESPACE}" PARENT_SCOPE)
        message(STATUS "  namespace: ${CONFIG_NAMESPACE}")
    endif()
endfunction()

# 自动应用构建策略
function(auto_apply_build_strategy)
    cmake_parse_arguments(
        AUTO_APPLY
        ""
        "PROJECT_DIR"
        ""
        ${ARGN}
    )
    
    if(NOT AUTO_APPLY_PROJECT_DIR)
        set(AUTO_APPLY_PROJECT_DIR "${CMAKE_SOURCE_DIR}")
    endif()
    
    message(STATUS "Applying auto build strategy...")
    
    # 先加载配置
    auto_load_project_config(PROJECT_DIR "${AUTO_APPLY_PROJECT_DIR}")
    
    # 应用 C 标准
    if(MODULE_BUILD_CONFIG_C_STANDARD)
        set(CMAKE_C_STANDARD "${MODULE_BUILD_CONFIG_C_STANDARD}" PARENT_SCOPE)
        set(CMAKE_C_STANDARD_REQUIRED ON PARENT_SCOPE)
        message(STATUS "Set C standard to: ${MODULE_BUILD_CONFIG_C_STANDARD}")
    endif()
    
    # 应用输出目录
    if(MODULE_BUILD_CONFIG_OUTPUT_DIR)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${AUTO_APPLY_PROJECT_DIR}/${MODULE_BUILD_CONFIG_OUTPUT_DIR}" PARENT_SCOPE)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${AUTO_APPLY_PROJECT_DIR}/${MODULE_BUILD_CONFIG_OUTPUT_DIR}" PARENT_SCOPE)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${AUTO_APPLY_PROJECT_DIR}/${MODULE_BUILD_CONFIG_OUTPUT_DIR}" PARENT_SCOPE)
        message(STATUS "Set output directory to: ${MODULE_BUILD_CONFIG_OUTPUT_DIR}")
    endif()
    
    message(STATUS "Auto build strategy applied successfully!")
endfunction()

# 项目类型标识定义
set(PROJECT_TYPES "idcu_agent" "c_shared_library" "c_static_library" "c_executable" "custom")

# 跨平台检测和配置
if(WIN32)
    set(PLATFORM_NAME "windows")
    set(PLATFORM_FAMILY "windows")
    set(LIB_PREFIX "")
    set(LIB_SHARED_SUFFIX ".dll")
    set(LIB_STATIC_SUFFIX ".lib")
    set(EXE_SUFFIX ".exe")
    set(PLATFORM_PATH_SEPARATOR ";")
elseif(APPLE)
    set(PLATFORM_NAME "macos")
    set(PLATFORM_FAMILY "unix")
    set(LIB_PREFIX "lib")
    set(LIB_SHARED_SUFFIX ".dylib")
    set(LIB_STATIC_SUFFIX ".a")
    set(EXE_SUFFIX "")
    set(PLATFORM_PATH_SEPARATOR ":")
elseif(UNIX)
    set(PLATFORM_NAME "linux")
    set(PLATFORM_FAMILY "unix")
    set(LIB_PREFIX "lib")
    set(LIB_SHARED_SUFFIX ".so")
    set(LIB_STATIC_SUFFIX ".a")
    set(EXE_SUFFIX "")
    set(PLATFORM_PATH_SEPARATOR ":")
else()
    set(PLATFORM_NAME "unknown")
    set(PLATFORM_FAMILY "unknown")
    set(LIB_PREFIX "lib")
    set(LIB_SHARED_SUFFIX ".so")
    set(LIB_STATIC_SUFFIX ".a")
    set(EXE_SUFFIX "")
    set(PLATFORM_PATH_SEPARATOR ":")
endif()

# 架构检测
if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64|AMD64|x86_64|x64")
    set(ARCH_NAME "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|ARM64|aarch64|AARCH64")
    set(ARCH_NAME "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|ARM")
    set(ARCH_NAME "arm")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386|i686|x86")
    set(ARCH_NAME "x86")
else()
    set(ARCH_NAME "${CMAKE_SYSTEM_PROCESSOR}")
endif()

# 编译器检测
if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(COMPILER_NAME "msvc")
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(COMPILER_NAME "gcc")
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(COMPILER_NAME "clang")
elseif(CMAKE_C_COMPILER_ID STREQUAL "Intel")
    set(COMPILER_NAME "intel")
else()
    set(COMPILER_NAME "unknown")
endif()

# 初始化模块构建系统
function(module_build_init)
    cmake_parse_arguments(
        MODULE_BUILD_INIT
        ""
        "CONFIG_DIR;TEMPLATE_DIR"
        ""
        ${ARGN}
    )
    
    if(NOT MODULE_BUILD_INIT_CONFIG_DIR)
        get_filename_component(MODULE_BUILD_INIT_CONFIG_DIR "${CMAKE_CURRENT_LIST_DIR}/../config" ABSOLUTE)
    endif()
    
    set(MODULE_BUILD_CONFIG_DIR "${MODULE_BUILD_INIT_CONFIG_DIR}" CACHE INTERNAL "Module Build Config Directory")
    
    if(NOT MODULE_BUILD_INIT_TEMPLATE_DIR)
        get_filename_component(MODULE_BUILD_INIT_TEMPLATE_DIR "${CMAKE_CURRENT_LIST_DIR}/../templates" ABSOLUTE)
    endif()
    
    set(MODULE_BUILD_TEMPLATE_DIR "${MODULE_BUILD_INIT_TEMPLATE_DIR}" CACHE INTERNAL "Module Build Template Directory")
    
    message(STATUS "========================================")
    message(STATUS "Module Build System initialized")
    message(STATUS "========================================")
    message(STATUS "  Platform: ${PLATFORM_NAME} (${PLATFORM_FAMILY})")
    message(STATUS "  Architecture: ${ARCH_NAME}")
    message(STATUS "  Compiler: ${COMPILER_NAME}")
    message(STATUS "  Library prefix: ${LIB_PREFIX}")
    message(STATUS "  Shared library suffix: ${LIB_SHARED_SUFFIX}")
    message(STATUS "  Static library suffix: ${LIB_STATIC_SUFFIX}")
    message(STATUS "  Executable suffix: ${EXE_SUFFIX}")
    message(STATUS "  Config directory: ${MODULE_BUILD_CONFIG_DIR}")
    message(STATUS "========================================")
endfunction()

# 加载默认配置
function(module_build_load_config)
    cmake_parse_arguments(
        MODULE_BUILD_LOAD_CONFIG
        ""
        "CONFIG_FILE"
        ""
        ${ARGN}
    )
    
    if(NOT MODULE_BUILD_LOAD_CONFIG_CONFIG_FILE)
        set(MODULE_BUILD_LOAD_CONFIG_CONFIG_FILE "${MODULE_BUILD_CONFIG_DIR}/default.yaml")
    endif()
    
    if(EXISTS "${MODULE_BUILD_LOAD_CONFIG_CONFIG_FILE}")
        message(STATUS "Loading config from: ${MODULE_BUILD_LOAD_CONFIG_CONFIG_FILE}")
    else()
        message(WARNING "Config file not found: ${MODULE_BUILD_LOAD_CONFIG_CONFIG_FILE}")
    endif()
endfunction()

# 自动检测项目类型
function(module_build_auto_detect_project)
    cmake_parse_arguments(
        MODULE_BUILD_AUTO_DETECT
        ""
        "PROJECT_DIR;RESULT_VAR"
        ""
        ${ARGN}
    )
    
    if(NOT MODULE_BUILD_AUTO_DETECT_PROJECT_DIR)
        set(MODULE_BUILD_AUTO_DETECT_PROJECT_DIR "${CMAKE_SOURCE_DIR}")
    endif()
    
    set(DETECTED_PROJECT_TYPE "custom")
    
    # 检测 IDCU Agent 项目
    if(EXISTS "${MODULE_BUILD_AUTO_DETECT_PROJECT_DIR}/config/agent.cfg" AND
       EXISTS "${MODULE_BUILD_AUTO_DETECT_PROJECT_DIR}/modules/core/sdk/")
        set(DETECTED_PROJECT_TYPE "idcu_agent")
    # 检测 C 可执行项目
    elseif(EXISTS "${MODULE_BUILD_AUTO_DETECT_PROJECT_DIR}/src/main.c")
        set(DETECTED_PROJECT_TYPE "c_executable")
    # 检测通用 C 项目
    elseif(EXISTS "${MODULE_BUILD_AUTO_DETECT_PROJECT_DIR}/CMakeLists.txt" AND
           EXISTS "${MODULE_BUILD_AUTO_DETECT_PROJECT_DIR}/include/")
        set(DETECTED_PROJECT_TYPE "c_shared_library")
    endif()
    
    if(MODULE_BUILD_AUTO_DETECT_RESULT_VAR)
        set(${MODULE_BUILD_AUTO_DETECT_RESULT_VAR} "${DETECTED_PROJECT_TYPE}" PARENT_SCOPE)
    endif()
    
    message(STATUS "Auto-detected project type: ${DETECTED_PROJECT_TYPE}")
endfunction()

# 构建模块（通用核心函数）
function(build_module)
    cmake_parse_arguments(
        BUILD_MODULE
        ""
        "MODULE_DIR;NAMESPACE;INSTALL_NAMESPACE"
        "ALIAS_PREFIXES"
        ${ARGN}
    )

    if(NOT BUILD_MODULE_MODULE_DIR)
        set(BUILD_MODULE_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    set(MODULE_JSON_PATH "${BUILD_MODULE_MODULE_DIR}/module.json")

    if(NOT EXISTS "${MODULE_JSON_PATH}")
        message(FATAL_ERROR "module.json not found in ${BUILD_MODULE_MODULE_DIR}")
    endif()

    # 读取 module.json 内容
    file(READ "${MODULE_JSON_PATH}" MODULE_JSON_CONTENT)

    # 从 module.json 加载插件配置（暂时跳过）
    # load_plugins_from_config(MODULE_DIR "${BUILD_MODULE_MODULE_DIR}")

    # 先处理依赖（暂时跳过）
    # process_module_dependencies(
    #     MODULE_DIR "${BUILD_MODULE_MODULE_DIR}"
    #     NAMESPACE "${BUILD_MODULE_NAMESPACE}"
    # )

    # 尝试从新结构提取配置
    safe_json_get(MODULE_CONFIG "${MODULE_JSON_CONTENT}" KEYS "module")
    set(HAS_MODULE_CONFIG OFF)
    if(MODULE_CONFIG)
        set(HAS_MODULE_CONFIG ON)
    endif()

    # 提取模块名称和版本 - 支持新旧两种格式
    if(HAS_MODULE_CONFIG)
        safe_json_get(MODULE_NAME "${MODULE_CONFIG}" KEYS "name")
        safe_json_get(MODULE_VERSION "${MODULE_CONFIG}" KEYS "version")
        if(NOT MODULE_VERSION)
            set(MODULE_VERSION "1.0.0")
        endif()
        
        # 检查 auto_adapt 标志
        safe_json_get(AUTO_ADAPT_VALUE "${MODULE_CONFIG}" KEYS "auto_adapt")
        if(AUTO_ADAPT_VALUE)
            message(STATUS "Auto-adapt enabled for module: ${MODULE_NAME}")
        endif()
    else()
        safe_json_get(MODULE_NAME "${MODULE_JSON_CONTENT}" KEYS "name")
        safe_json_get(MODULE_VERSION "${MODULE_JSON_CONTENT}" KEYS "version")
        if(NOT MODULE_VERSION)
            set(MODULE_VERSION "1.0.0")
        endif()
    endif()

    # 设置项目
    cmake_minimum_required(VERSION 3.10)
    project(${MODULE_NAME} VERSION ${MODULE_VERSION} LANGUAGES C)

    # 执行 pre_build 钩子
    execute_hook(
        HOOK_NAME "pre_build"
        MODULE_DIR "${BUILD_MODULE_MODULE_DIR}"
        MODULE_NAME "${MODULE_NAME}"
        MODULE_VERSION "${MODULE_VERSION}"
    )

    # 检查是否有 build 配置
    safe_json_get(BUILD_CONFIG "${MODULE_JSON_CONTENT}" KEYS "build")
    set(HAS_BUILD_CONFIG OFF)
    set(BUILD_CONFIG "")
    if(BUILD_CONFIG)
        set(HAS_BUILD_CONFIG ON)
    endif()

    # 设置默认构建配置
    set(LANG_TYPE "C")
    set(LANG_STANDARD "99")
    set(LIB_TYPE "static")
    set(LINK_LIBRARIES "")
    set(PLATFORM_DEPS "")
    set(SRC_PATTERNS "")

    if(HAS_BUILD_CONFIG)
        # 处理 c_standard (支持 auto)
        safe_json_get(LANG_STANDARD_VALUE "${BUILD_CONFIG}" KEYS "c_standard")
        if(NOT LANG_STANDARD_VALUE)
            safe_json_get(LANG_STANDARD_VALUE "${BUILD_CONFIG}" KEYS "standard")
        endif()
        if(LANG_STANDARD_VALUE)
            if(LANG_STANDARD_VALUE STREQUAL "auto")
                # 自动检测：优先用 c11，否则用 c99
                if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|Intel")
                    set(LANG_STANDARD "11")
                else()
                    set(LANG_STANDARD "99")
                endif()
                message(STATUS "Auto-detected C standard: ${LANG_STANDARD}")
            else()
                set(LANG_STANDARD "${LANG_STANDARD_VALUE}")
            endif()
        endif()
        
        # 处理 src (支持 auto)
        safe_json_get(SRC_VALUE "${BUILD_CONFIG}" KEYS "src")
        if(SRC_VALUE)
            if(SRC_VALUE STREQUAL "auto")
                # 自动扫描：默认扫描 src 目录下的所有 .c 文件
                set(SRC_PATTERNS "${BUILD_MODULE_MODULE_DIR}/src/*.c")
                message(STATUS "Auto-scanning source files from: src/")
            else()
                set(SRC_PATTERNS "${BUILD_MODULE_MODULE_DIR}/${SRC_VALUE}")
            endif()
        endif()
        
        safe_json_get(LANG_TYPE "${BUILD_CONFIG}" KEYS "language")
        if(NOT LANG_TYPE)
            set(LANG_TYPE "C")
        endif()
        
        safe_json_get(LIB_TYPE "${BUILD_CONFIG}" KEYS "type")
        if(NOT LIB_TYPE)
            set(LIB_TYPE "static")
        endif()
        
        safe_json_get(LINK_LIBS "${BUILD_CONFIG}" KEYS "deps")
        if(NOT LINK_LIBS)
            safe_json_get(LINK_LIBS "${BUILD_CONFIG}" KEYS "link_libraries")
        endif()
        if(LINK_LIBS)
            string(JSON LINK_LIBS_COUNT LENGTH "${LINK_LIBS}")
            if(LINK_LIBS_COUNT GREATER 0)
                math(EXPR LINK_LIBS_COUNT "${LINK_LIBS_COUNT} - 1")
                foreach(INDEX RANGE ${LINK_LIBS_COUNT})
                    string(JSON LIB_NAME GET "${LINK_LIBS}" ${INDEX})
                    list(APPEND LINK_LIBRARIES ${LIB_NAME})
                endforeach()
            endif()
        endif()
    endif()

    # 设置 C 标准
    set(CMAKE_C_STANDARD ${LANG_STANDARD})
    set(CMAKE_C_STANDARD_REQUIRED ON)

    # 收集源文件
    if(SRC_PATTERNS)
        file(GLOB_RECURSE SRCS
            ${SRC_PATTERNS}
        )
    else()
        file(GLOB_RECURSE SRCS
            "${BUILD_MODULE_MODULE_DIR}/src/*.c"
        )
    endif()

    # 创建库或可执行文件
    set(IS_EXECUTABLE OFF)
    set(IS_SHARED_LIBRARY OFF)
    
    if(HAS_BUILD_CONFIG)
        safe_json_get(BUILD_TYPE "${BUILD_CONFIG}" KEYS "type")
        if(BUILD_TYPE)
            if(BUILD_TYPE STREQUAL "executable")
                add_executable(${MODULE_NAME} ${SRCS})
                set(IS_EXECUTABLE ON)
            elseif(BUILD_TYPE STREQUAL "shared")
                add_library(${MODULE_NAME} SHARED ${SRCS})
                set(IS_SHARED_LIBRARY ON)
            elseif(BUILD_TYPE STREQUAL "static")
                add_library(${MODULE_NAME} STATIC ${SRCS})
            else()
                add_library(${MODULE_NAME} STATIC ${SRCS})
            endif()
        else()
            if(LIB_TYPE STREQUAL "static")
                add_library(${MODULE_NAME} STATIC ${SRCS})
            elseif(LIB_TYPE STREQUAL "shared")
                add_library(${MODULE_NAME} SHARED ${SRCS})
                set(IS_SHARED_LIBRARY ON)
            else()
                add_library(${MODULE_NAME} STATIC ${SRCS})
            endif()
        endif()
    else()
        if(LIB_TYPE STREQUAL "static")
            add_library(${MODULE_NAME} STATIC ${SRCS})
        elseif(LIB_TYPE STREQUAL "shared")
            add_library(${MODULE_NAME} SHARED ${SRCS})
            set(IS_SHARED_LIBRARY ON)
        else()
            add_library(${MODULE_NAME} STATIC ${SRCS})
        endif()
    endif()

    # 跨平台编译选项
    if(WIN32)
        # Windows 特定选项
        target_compile_definitions(${MODULE_NAME} PRIVATE
            _CRT_SECURE_NO_WARNINGS
            _CRT_NONSTDC_NO_DEPRECATE
            WIN32_LEAN_AND_MEAN
        )
        
        if(MSVC)
            # MSVC 特定编译选项
            target_compile_options(${MODULE_NAME} PRIVATE
                /W3
                /utf-8
            )
            if(NOT IS_EXECUTABLE)
                target_compile_options(${MODULE_NAME} PRIVATE
                    /EHsc
                )
            endif()
        elseif(MINGW)
            # MinGW 特定编译选项
            target_compile_options(${MODULE_NAME} PRIVATE
                -Wall
                -Wextra
                -Wno-unused-parameter
            )
        endif()
        
        # Windows 动态库导出定义
        if(IS_SHARED_LIBRARY)
            string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)
            target_compile_definitions(${MODULE_NAME} PRIVATE
                ${MODULE_NAME_UPPER}_EXPORTS
            )
        endif()
        
    elseif(APPLE)
        # macOS 特定选项
        set_target_properties(${MODULE_NAME} PROPERTIES
            MACOSX_RPATH ON
            INSTALL_NAME_DIR "@rpath"
        )
        
        target_compile_options(${MODULE_NAME} PRIVATE
            -Wall
            -Wextra
            -Wno-unused-parameter
        )
        
        if(NOT IS_EXECUTABLE)
            set_target_properties(${MODULE_NAME} PROPERTIES
                BUILD_WITH_INSTALL_RPATH ON
            )
        endif()
        
    elseif(UNIX)
        # Linux/Unix 特定选项
        target_compile_options(${MODULE_NAME} PRIVATE
            -Wall
            -Wextra
            -Wno-unused-parameter
            -fPIC
        )
        
        if(NOT IS_EXECUTABLE)
            set_target_properties(${MODULE_NAME} PROPERTIES
                POSITION_INDEPENDENT_CODE ON
                INSTALL_RPATH "$ORIGIN"
                BUILD_WITH_INSTALL_RPATH ON
            )
        endif()
    endif()

    # 添加别名（支持命名空间）
    if(BUILD_MODULE_NAMESPACE AND NOT IS_EXECUTABLE)
        set(MODULE_SHORT_NAMES "")
        
        if(BUILD_MODULE_ALIAS_PREFIXES)
            foreach(PREFIX ${BUILD_MODULE_ALIAS_PREFIXES})
                string(REGEX REPLACE "^${PREFIX}[-_]" "" SHORT_NAME "${MODULE_NAME}")
                if(NOT SHORT_NAME STREQUAL MODULE_NAME)
                    list(APPEND MODULE_SHORT_NAMES "${SHORT_NAME}")
                endif()
            endforeach()
        else()
            list(APPEND MODULE_SHORT_NAMES "${MODULE_NAME}")
        endif()
        
        foreach(SHORT_NAME ${MODULE_SHORT_NAMES})
            if(NOT TARGET ${BUILD_MODULE_NAMESPACE}::${SHORT_NAME})
                add_library(${BUILD_MODULE_NAMESPACE}::${SHORT_NAME} ALIAS ${MODULE_NAME})
            endif()
        endforeach()
    endif()

    # 公共头文件目录
    if(HAS_BUILD_CONFIG)
        safe_json_get(INCLUDES_GET "${BUILD_CONFIG}" KEYS "includes")
        if(INCLUDES_GET)
            string(JSON INCLUDES_COUNT LENGTH "${INCLUDES_GET}")
            if(INCLUDES_COUNT GREATER 0)
                math(EXPR INCLUDES_COUNT "${INCLUDES_COUNT} - 1")
                foreach(INDEX RANGE ${INCLUDES_COUNT})
                    string(JSON INCLUDE_PATH GET "${INCLUDES_GET}" ${INDEX})
                    target_include_directories(${MODULE_NAME} PUBLIC
                        $<BUILD_INTERFACE:${BUILD_MODULE_MODULE_DIR}/${INCLUDE_PATH}>
                    )
                endforeach()
            endif()
        else()
            target_include_directories(${MODULE_NAME} PUBLIC
                $<BUILD_INTERFACE:${BUILD_MODULE_MODULE_DIR}/include>
                $<INSTALL_INTERFACE:include>
            )
        endif()
    else()
        target_include_directories(${MODULE_NAME} PUBLIC
            $<BUILD_INTERFACE:${BUILD_MODULE_MODULE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        )
    endif()

    # 链接依赖
    foreach(DEP_NAME ${LINK_LIBRARIES})
        if(BUILD_MODULE_NAMESPACE)
            if(TARGET ${BUILD_MODULE_NAMESPACE}::${DEP_NAME})
                target_link_libraries(${MODULE_NAME} PUBLIC ${BUILD_MODULE_NAMESPACE}::${DEP_NAME})
            elseif(TARGET ${DEP_NAME})
                target_link_libraries(${MODULE_NAME} PUBLIC ${DEP_NAME})
            endif()
        elseif(TARGET ${DEP_NAME})
            target_link_libraries(${MODULE_NAME} PUBLIC ${DEP_NAME})
        endif()
    endforeach()

    # 处理平台依赖
    if(HAS_BUILD_CONFIG)
        safe_json_get(PLATFORM_DEPS "${BUILD_CONFIG}" KEYS "platform_deps")
        if(PLATFORM_DEPS)
            if(WIN32)
                safe_json_get(WIN32_LIBS "${PLATFORM_DEPS}" KEYS "win32")
                if(WIN32_LIBS)
                    string(JSON WIN32_LIB_COUNT LENGTH "${WIN32_LIBS}")
                    if(WIN32_LIB_COUNT GREATER 0)
                        math(EXPR WIN32_LIB_COUNT "${WIN32_LIB_COUNT} - 1")
                        foreach(LIB_INDEX RANGE ${WIN32_LIB_COUNT})
                            string(JSON LIB_NAME GET "${WIN32_LIBS}" ${LIB_INDEX})
                            target_link_libraries(${MODULE_NAME} PUBLIC ${LIB_NAME})
                        endforeach()
                    endif()
                endif()
            else()
                safe_json_get(UNIX_LIBS "${PLATFORM_DEPS}" KEYS "unix")
                if(UNIX_LIBS)
                    string(JSON UNIX_LIB_COUNT LENGTH "${UNIX_LIBS}")
                    if(UNIX_LIB_COUNT GREATER 0)
                        math(EXPR UNIX_LIB_COUNT "${UNIX_LIB_COUNT} - 1")
                        foreach(LIB_INDEX RANGE ${UNIX_LIB_COUNT})
                            string(JSON LIB_NAME GET "${UNIX_LIBS}" ${LIB_INDEX})
                            target_link_libraries(${MODULE_NAME} PUBLIC ${LIB_NAME})
                        endforeach()
                    endif()
                endif()
            endif()
        endif()
    endif()

    # 安装配置（仅库）
    if((NOT IS_EXECUTABLE) AND HAS_BUILD_CONFIG)
        include(GNUInstallDirs)
        include(CMakePackageConfigHelpers)

        install(TARGETS ${MODULE_NAME}
            EXPORT ${MODULE_NAME}-targets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

        if(EXISTS "${BUILD_MODULE_MODULE_DIR}/include")
            install(DIRECTORY "${BUILD_MODULE_MODULE_DIR}/include/"
                DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            )
        endif()

        if(NOT BUILD_MODULE_INSTALL_NAMESPACE)
            set(BUILD_MODULE_INSTALL_NAMESPACE "${BUILD_MODULE_NAMESPACE}")
        endif()
        
        if(BUILD_MODULE_INSTALL_NAMESPACE)
            install(EXPORT ${MODULE_NAME}-targets
                FILE ${MODULE_NAME}-targets.cmake
                NAMESPACE ${BUILD_MODULE_INSTALL_NAMESPACE}::
                DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${MODULE_NAME}
            )
        else()
            install(EXPORT ${MODULE_NAME}-targets
                FILE ${MODULE_NAME}-targets.cmake
                DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${MODULE_NAME}
            )
        endif()

        write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config-version.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY SameMajorVersion
        )

        set(CONFIG_CONTENT "include(CMakeFindDependencyMacro)\n")
        foreach(DEP_NAME ${LINK_LIBRARIES})
            string(APPEND CONFIG_CONTENT "find_dependency(${DEP_NAME})\n")
        endforeach()
        string(APPEND CONFIG_CONTENT "include(\"\${CMAKE_CURRENT_LIST_DIR}/${MODULE_NAME}-targets.cmake\")\n")

        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config.cmake" "${CONFIG_CONTENT}")

        install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config-version.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${MODULE_NAME}
        )
    endif()

    # 构建测试
    if(HAS_BUILD_CONFIG)
        if(NOT DEFINED BUILD_TESTS)
            option(BUILD_TESTS "Build tests" OFF)
        endif()
        if(BUILD_TESTS AND EXISTS "${BUILD_MODULE_MODULE_DIR}/tests")
            enable_testing()
            file(GLOB TEST_SRCS "${BUILD_MODULE_MODULE_DIR}/tests/test_*.c")
            foreach(test_src ${TEST_SRCS})
                get_filename_component(test_name ${test_src} NAME_WE)
                add_executable(${MODULE_NAME}_${test_name} ${test_src})
                if(BUILD_MODULE_NAMESPACE AND MODULE_SHORT_NAMES)
                    list(GET MODULE_SHORT_NAMES 0 FIRST_SHORT_NAME)
                    target_link_libraries(${MODULE_NAME}_${test_name} PRIVATE ${BUILD_MODULE_NAMESPACE}::${FIRST_SHORT_NAME})
                else()
                    target_link_libraries(${MODULE_NAME}_${test_name} PRIVATE ${MODULE_NAME})
                endif()
                add_test(NAME ${MODULE_NAME}_${test_name} COMMAND ${MODULE_NAME}_${test_name})
            endforeach()
        endif()

        if(NOT DEFINED BUILD_EXAMPLES)
            option(BUILD_EXAMPLES "Build examples" OFF)
        endif()
        if(BUILD_EXAMPLES AND EXISTS "${BUILD_MODULE_MODULE_DIR}/examples")
            file(GLOB EXAMPLE_SRCS "${BUILD_MODULE_MODULE_DIR}/examples/example_*.c")
            foreach(example_src ${EXAMPLE_SRCS})
                get_filename_component(example_name ${example_src} NAME_WE)
                add_executable(${MODULE_NAME}_${example_name} ${example_src})
                if(BUILD_MODULE_NAMESPACE AND MODULE_SHORT_NAMES)
                    list(GET MODULE_SHORT_NAMES 0 FIRST_SHORT_NAME)
                    target_link_libraries(${MODULE_NAME}_${example_name} PRIVATE ${BUILD_MODULE_NAMESPACE}::${FIRST_SHORT_NAME})
                else()
                    target_link_libraries(${MODULE_NAME}_${example_name} PRIVATE ${MODULE_NAME})
                endif()
            endforeach()
        endif()
    endif()

    # 产物标准化输出
    auto_package_module(
        MODULE_DIR "${BUILD_MODULE_MODULE_DIR}"
        MODULE_NAME "${MODULE_NAME}"
        MODULE_VERSION "${MODULE_VERSION}"
    )

    # 执行 post_build 钩子
    execute_hook(
        HOOK_NAME "post_build"
        MODULE_DIR "${BUILD_MODULE_MODULE_DIR}"
        MODULE_NAME "${MODULE_NAME}"
        MODULE_VERSION "${MODULE_VERSION}"
    )

endfunction()

# 检查项目类型是否有效
function(is_valid_project_type TYPE RESULT_VAR)
    if("${TYPE}" IN_LIST PROJECT_TYPES)
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

# 获取所有支持的项目类型
function(get_supported_project_types RESULT_VAR)
    set(${RESULT_VAR} "${PROJECT_TYPES}" PARENT_SCOPE)
endfunction()

# 设置项目类型
function(set_project_type TYPE)
    is_valid_project_type("${TYPE}" IS_VALID)
    if(NOT IS_VALID)
        message(WARNING "Invalid project type: ${TYPE}. Using 'custom' instead.")
        set(TYPE "custom")
    endif()
    set(MODULE_BUILD_PROJECT_TYPE "${TYPE}" PARENT_SCOPE)
    message(STATUS "Project type set to: ${TYPE}")
endfunction()

# 运行完整构建流程
function(module_build_run)
    module_build_auto_detect_project()
    build_module(${ARGN})
endfunction()

# ==============================================================================
# 产物标准化输出功能
# ==============================================================================

# 生成 build-info.json 文件
function(_generate_build_info)
    cmake_parse_arguments(
        GEN_BUILD_INFO
        ""
        "OUTPUT_DIR;MODULE_NAME;MODULE_VERSION"
        ""
        ${ARGN}
    )

    set(BUILD_INFO_PATH "${GEN_BUILD_INFO_OUTPUT_DIR}/build-info.json")
    
    # 获取当前时间
    string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%dT%H:%M:%SZ" UTC)
    
    # 获取Git信息（如果可用）
    set(GIT_COMMIT "unknown")
    set(GIT_BRANCH "unknown")
    if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
        find_package(Git QUIET)
        if(GIT_FOUND)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                OUTPUT_VARIABLE GIT_COMMIT
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                OUTPUT_VARIABLE GIT_BRANCH
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        endif()
    endif()
    
    # 构建 build-info.json 内容
    set(BUILD_INFO_CONTENT "{\n")
    string(APPEND BUILD_INFO_CONTENT "  \"module\": {\n")
    string(APPEND BUILD_INFO_CONTENT "    \"name\": \"${GEN_BUILD_INFO_MODULE_NAME}\",\n")
    string(APPEND BUILD_INFO_CONTENT "    \"version\": \"${GEN_BUILD_INFO_MODULE_VERSION}\"\n")
    string(APPEND BUILD_INFO_CONTENT "  },\n")
    string(APPEND BUILD_INFO_CONTENT "  \"build\": {\n")
    string(APPEND BUILD_INFO_CONTENT "    \"timestamp\": \"${BUILD_TIMESTAMP}\",\n")
    string(APPEND BUILD_INFO_CONTENT "    \"platform\": \"${PLATFORM_NAME}\",\n")
    string(APPEND BUILD_INFO_CONTENT "    \"arch\": \"${ARCH_NAME}\",\n")
    string(APPEND BUILD_INFO_CONTENT "    \"compiler\": \"${COMPILER_NAME}\",\n")
    string(APPEND BUILD_INFO_CONTENT "    \"cmake_version\": \"${CMAKE_VERSION}\"\n")
    string(APPEND BUILD_INFO_CONTENT "  },\n")
    string(APPEND BUILD_INFO_CONTENT "  \"git\": {\n")
    string(APPEND BUILD_INFO_CONTENT "    \"commit\": \"${GIT_COMMIT}\",\n")
    string(APPEND BUILD_INFO_CONTENT "    \"branch\": \"${GIT_BRANCH}\"\n")
    string(APPEND BUILD_INFO_CONTENT "  }\n")
    string(APPEND BUILD_INFO_CONTENT "}\n")
    
    # 写入文件
    file(WRITE "${BUILD_INFO_PATH}" "${BUILD_INFO_CONTENT}")
    message(STATUS "Generated build-info.json: ${BUILD_INFO_PATH}")
endfunction()

# 标准化产物输出
function(package_module)
    cmake_parse_arguments(
        PACKAGE_MODULE
        ""
        "MODULE_DIR;MODULE_NAME;MODULE_VERSION;OUTPUT_ROOT"
        ""
        ${ARGN}
    )

    if(NOT PACKAGE_MODULE_MODULE_DIR)
        set(PACKAGE_MODULE_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    if(NOT PACKAGE_MODULE_MODULE_NAME)
        message(FATAL_ERROR "MODULE_NAME is required for package_module")
    endif()

    if(NOT PACKAGE_MODULE_MODULE_VERSION)
        set(PACKAGE_MODULE_MODULE_VERSION "1.0.0")
    endif()

    if(NOT PACKAGE_MODULE_OUTPUT_ROOT)
        set(PACKAGE_MODULE_OUTPUT_ROOT "${CMAKE_BINARY_DIR}/output")
    endif()

    # 执行 pre_package 钩子
    execute_hook(
        HOOK_NAME "pre_package"
        MODULE_DIR "${PACKAGE_MODULE_MODULE_DIR}"
        MODULE_NAME "${PACKAGE_MODULE_MODULE_NAME}"
        MODULE_VERSION "${PACKAGE_MODULE_MODULE_VERSION}"
    )

    # 创建输出目录结构：{module}-{version}-{platform}-{arch}/
    set(PACKAGE_NAME "${PACKAGE_MODULE_MODULE_NAME}-${PACKAGE_MODULE_MODULE_VERSION}-${PLATFORM_NAME}-${ARCH_NAME}")
    set(PACKAGE_OUTPUT_DIR "${PACKAGE_MODULE_OUTPUT_ROOT}/${PACKAGE_NAME}")
    set(PACKAGE_BIN_DIR "${PACKAGE_OUTPUT_DIR}/bin")
    set(PACKAGE_LIB_DIR "${PACKAGE_OUTPUT_DIR}/lib")
    set(PACKAGE_META_DIR "${PACKAGE_OUTPUT_DIR}/meta")

    # 创建目录
    file(MAKE_DIRECTORY "${PACKAGE_OUTPUT_DIR}")
    file(MAKE_DIRECTORY "${PACKAGE_BIN_DIR}")
    file(MAKE_DIRECTORY "${PACKAGE_LIB_DIR}")
    file(MAKE_DIRECTORY "${PACKAGE_META_DIR}")

    message(STATUS "Packaging module: ${PACKAGE_MODULE_MODULE_NAME} v${PACKAGE_MODULE_MODULE_VERSION}")
    message(STATUS "Output directory: ${PACKAGE_OUTPUT_DIR}")

    # 复制 module.json 到 meta/ 目录
    set(MODULE_JSON_SOURCE "${PACKAGE_MODULE_MODULE_DIR}/module.json")
    if(EXISTS "${MODULE_JSON_SOURCE}")
        configure_file("${MODULE_JSON_SOURCE}" "${PACKAGE_META_DIR}/module.json" COPYONLY)
        message(STATUS "Copied module.json to meta/ directory")
    endif()

    # 生成 build-info.json
    _generate_build_info(
        OUTPUT_DIR "${PACKAGE_OUTPUT_DIR}"
        MODULE_NAME "${PACKAGE_MODULE_MODULE_NAME}"
        MODULE_VERSION "${PACKAGE_MODULE_MODULE_VERSION}"
    )

    # 设置安装目标
    set_target_properties(${PACKAGE_MODULE_MODULE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PACKAGE_BIN_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${PACKAGE_LIB_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY "${PACKAGE_LIB_DIR}"
    )

    # 添加自定义目标用于后期打包（可扩展）
    add_custom_target(package_${PACKAGE_MODULE_MODULE_NAME}
        COMMAND ${CMAKE_COMMAND} -E echo "Module ${PACKAGE_MODULE_MODULE_NAME} packaged successfully at: ${PACKAGE_OUTPUT_DIR}"
        DEPENDS ${PACKAGE_MODULE_MODULE_NAME}
        COMMENT "Packaging module: ${PACKAGE_MODULE_MODULE_NAME}"
    )

    message(STATUS "Module packaging configured: ${PACKAGE_NAME}")

    # 执行 post_package 钩子
    execute_hook(
        HOOK_NAME "post_package"
        MODULE_DIR "${PACKAGE_MODULE_MODULE_DIR}"
        MODULE_NAME "${PACKAGE_MODULE_MODULE_NAME}"
        MODULE_VERSION "${PACKAGE_MODULE_MODULE_VERSION}"
    )
endfunction()

# 简化版：在 build_module 之后自动调用
function(auto_package_module)
    cmake_parse_arguments(
        AUTO_PACKAGE
        ""
        "MODULE_DIR;MODULE_NAME;MODULE_VERSION;OUTPUT_ROOT"
        ""
        ${ARGN}
    )

    if(NOT AUTO_PACKAGE_MODULE_NAME)
        message(FATAL_ERROR "MODULE_NAME is required for auto_package_module")
    endif()

    # 调用 package_module
    package_module(
        MODULE_DIR "${AUTO_PACKAGE_MODULE_DIR}"
        MODULE_NAME "${AUTO_PACKAGE_MODULE_NAME}"
        MODULE_VERSION "${AUTO_PACKAGE_MODULE_VERSION}"
        OUTPUT_ROOT "${AUTO_PACKAGE_OUTPUT_ROOT}"
    )
endfunction()
