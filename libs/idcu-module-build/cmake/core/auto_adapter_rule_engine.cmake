# Auto Adapter Rule Engine - 自动适配规则引擎
# 负责自动检测项目类型和加载配置

include("${CMAKE_CURRENT_LIST_DIR}/json_utils.cmake")

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
