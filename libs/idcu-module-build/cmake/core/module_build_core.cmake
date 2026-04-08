# Module Build Core - 通用模块构建核心
# 基于 module.json 自动构建 C 语言模块
# 纯通用核心，聚合所有子模块

include(CMakeParseArguments)

# 包含所有子模块
include("${CMAKE_CURRENT_LIST_DIR}/json_utils.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/platform_detection.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/module_build_initializer.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/module_config_parser.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/source_collector.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/target_builder.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/auto_adapter_rule_engine.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/product_output.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/module_dependency_manager.cmake")

# 构建模块（主函数）
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

    # 1. 解析模块基本信息
    parse_module_info("${MODULE_JSON_PATH}"
        NAME_VAR MODULE_NAME
        VERSION_VAR MODULE_VERSION
        AUTO_ADAPT_VAR AUTO_ADAPT_VALUE
    )

    message(STATUS "正在构建模块: ${MODULE_NAME} v${MODULE_VERSION}")

    # 设置项目
    cmake_minimum_required(VERSION 3.10)
    project(${MODULE_NAME} VERSION ${MODULE_VERSION} LANGUAGES C)

    # 2. 解析构建配置
    parse_build_config("${MODULE_JSON_PATH}"
        LANG_TYPE_VAR LANG_TYPE
        LANG_STANDARD_VAR LANG_STANDARD
        LIB_TYPE_VAR LIB_TYPE
        SRC_PATTERNS_VAR SRC_PATTERNS
        LINK_LIBS_VAR LINK_LIBRARIES
        INCLUDES_VAR INCLUDES
        PLATFORM_DEPS_VAR PLATFORM_DEPS
    )

    # 3. 收集源文件
    if(SRC_PATTERNS)
        collect_sources("${BUILD_MODULE_MODULE_DIR}"
            SRCS_PATTERNS_VAR SRC_PATTERNS
        )
    else()
        collect_sources("${BUILD_MODULE_MODULE_DIR}")
    endif()
    set(SRCS "${COLLECTED_SOURCES}")

    # 4. 创建并配置构建目标
    create_build_target("${MODULE_NAME}" "${BUILD_MODULE_MODULE_DIR}"
        LIB_TYPE "${LIB_TYPE}"
        LANG_STANDARD "${LANG_STANDARD}"
        ALIAS_NAMESPACE "${BUILD_MODULE_NAMESPACE}"
        ALIAS_PREFIXES "${BUILD_MODULE_ALIAS_PREFIXES}"
        SOURCES "${SRCS}"
        LINK_LIBS "${LINK_LIBRARIES}"
        INCLUDES "${INCLUDES}"
        PLATFORM_DEPS "${PLATFORM_DEPS}"
    )

    message(STATUS "模块 ${MODULE_NAME} 配置完成!")

    # 5. 产物标准化输出
    auto_package_module(
        MODULE_DIR "${BUILD_MODULE_MODULE_DIR}"
        MODULE_NAME "${MODULE_NAME}"
        MODULE_VERSION "${MODULE_VERSION}"
    )

endfunction()

# 运行完整构建流程
function(module_build_run)
    module_build_auto_detect_project()
    build_module(${ARGN})
endfunction()
