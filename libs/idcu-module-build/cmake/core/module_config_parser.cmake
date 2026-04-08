# 模块配置解析器
# 负责从 module.json 读取和解析模块配置

include("${CMAKE_CURRENT_LIST_DIR}/json_utils.cmake")

# 从 module.json 读取模块基本信息
function(parse_module_info MODULE_JSON_PATH)
    cmake_parse_arguments(
        PARSE_INFO
        ""
        "NAME_VAR;VERSION_VAR;AUTO_ADAPT_VAR"
        ""
        ${ARGN}
    )

    file(READ "${MODULE_JSON_PATH}" MODULE_JSON_CONTENT)

    # 尝试从新结构提取配置
    safe_json_get(MODULE_CONFIG "${MODULE_JSON_CONTENT}" KEYS "module")
    set(HAS_MODULE_CONFIG OFF)
    if(MODULE_CONFIG)
        set(HAS_MODULE_CONFIG ON)
    endif()

    # 提取模块名称和版本 - 支持新旧两种格式
    set(MODULE_NAME "")
    set(MODULE_VERSION "1.0.0")
    set(AUTO_ADAPT_VALUE OFF)

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

    # 设置输出变量
    if(PARSE_INFO_NAME_VAR)
        set(${PARSE_INFO_NAME_VAR} "${MODULE_NAME}" PARENT_SCOPE)
    endif()
    if(PARSE_INFO_VERSION_VAR)
        set(${PARSE_INFO_VERSION_VAR} "${MODULE_VERSION}" PARENT_SCOPE)
    endif()
    if(PARSE_INFO_AUTO_ADAPT_VAR)
        set(${PARSE_INFO_AUTO_ADAPT_VAR} "${AUTO_ADAPT_VALUE}" PARENT_SCOPE)
    endif()

endfunction()

# 从 module.json 读取构建配置
function(parse_build_config MODULE_JSON_PATH)
    cmake_parse_arguments(
        PARSE_BUILD
        ""
        "LANG_TYPE_VAR;LANG_STANDARD_VAR;LIB_TYPE_VAR;SRC_PATTERNS_VAR"
        "LINK_LIBS_VAR;INCLUDES_VAR;PLATFORM_DEPS_VAR"
        ${ARGN}
    )

    file(READ "${MODULE_JSON_PATH}" MODULE_JSON_CONTENT)

    # 设置默认值
    set(LANG_TYPE "C")
    set(LANG_STANDARD "99")
    set(LIB_TYPE "static")
    set(LINK_LIBRARIES "")
    set(INCLUDES "")
    set(PLATFORM_DEPS "")
    set(SRC_PATTERNS "")

    # 检查是否有 build 配置
    safe_json_get(BUILD_CONFIG "${MODULE_JSON_CONTENT}" KEYS "build")
    set(HAS_BUILD_CONFIG OFF)
    if(BUILD_CONFIG)
        set(HAS_BUILD_CONFIG ON)
    endif()

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
                set(SRC_PATTERNS "src/*.c")
                message(STATUS "Auto-scanning source files from: src/")
            else()
                # 检查 src 是否是数组
                string(JSON SRC_TYPE TYPE "${SRC_VALUE}")
                if(SRC_TYPE STREQUAL "ARRAY")
                    # 是数组，逐个提取
                    string(JSON SRC_COUNT LENGTH "${SRC_VALUE}")
                    if(SRC_COUNT GREATER 0)
                        math(EXPR SRC_COUNT "${SRC_COUNT} - 1")
                        foreach(INDEX RANGE ${SRC_COUNT})
                            string(JSON PATTERN GET "${SRC_VALUE}" ${INDEX})
                            list(APPEND SRC_PATTERNS ${PATTERN})
                        endforeach()
                    endif()
                else()
                    # 不是数组，直接使用
                    set(SRC_PATTERNS "${SRC_VALUE}")
                endif()
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
        
        # 读取链接依赖
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

        # 读取头文件目录
        safe_json_get(INCLUDES_GET "${BUILD_CONFIG}" KEYS "includes")
        if(INCLUDES_GET)
            string(JSON INCLUDES_COUNT LENGTH "${INCLUDES_GET}")
            if(INCLUDES_COUNT GREATER 0)
                math(EXPR INCLUDES_COUNT "${INCLUDES_COUNT} - 1")
                foreach(INDEX RANGE ${INCLUDES_COUNT})
                    string(JSON INCLUDE_PATH GET "${INCLUDES_GET}" ${INDEX})
                    list(APPEND INCLUDES ${INCLUDE_PATH})
                endforeach()
            endif()
        endif()

        # 读取平台依赖
        safe_json_get(PLATFORM_DEPS "${BUILD_CONFIG}" KEYS "platform_deps")
    endif()

    # 设置输出变量
    if(PARSE_BUILD_LANG_TYPE_VAR)
        set(${PARSE_BUILD_LANG_TYPE_VAR} "${LANG_TYPE}" PARENT_SCOPE)
    endif()
    if(PARSE_BUILD_LANG_STANDARD_VAR)
        set(${PARSE_BUILD_LANG_STANDARD_VAR} "${LANG_STANDARD}" PARENT_SCOPE)
    endif()
    if(PARSE_BUILD_LIB_TYPE_VAR)
        set(${PARSE_BUILD_LIB_TYPE_VAR} "${LIB_TYPE}" PARENT_SCOPE)
    endif()
    if(PARSE_BUILD_SRC_PATTERNS_VAR)
        set(${PARSE_BUILD_SRC_PATTERNS_VAR} "${SRC_PATTERNS}" PARENT_SCOPE)
    endif()
    if(PARSE_BUILD_LINK_LIBS_VAR)
        set(${PARSE_BUILD_LINK_LIBS_VAR} "${LINK_LIBRARIES}" PARENT_SCOPE)
    endif()
    if(PARSE_BUILD_INCLUDES_VAR)
        set(${PARSE_BUILD_INCLUDES_VAR} "${INCLUDES}" PARENT_SCOPE)
    endif()
    if(PARSE_BUILD_PLATFORM_DEPS_VAR)
        set(${PARSE_BUILD_PLATFORM_DEPS_VAR} "${PLATFORM_DEPS}" PARENT_SCOPE)
    endif()

endfunction()
