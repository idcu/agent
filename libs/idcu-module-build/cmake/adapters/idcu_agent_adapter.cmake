# IDCU Agent 专属适配器
# IDCU Agent Specific Adapter

# 包含通用核心
include("${CMAKE_CURRENT_LIST_DIR}/../core/module_build_core.cmake")

# IDCU Agent 专用构建函数
function(idcu_build_module)
    cmake_parse_arguments(
        IDCU_MODULE
        ""
        "MODULE_DIR"
        ""
        ${ARGN}
    )

    if(NOT IDCU_MODULE_MODULE_DIR)
        set(IDCU_MODULE_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    set(MODULE_JSON_PATH "${IDCU_MODULE_MODULE_DIR}/module.json")

    if(NOT EXISTS "${MODULE_JSON_PATH}")
        message(FATAL_ERROR "module.json not found in ${IDCU_MODULE_MODULE_DIR}")
    endif()

    # 读取 module.json 内容
    file(READ "${MODULE_JSON_PATH}" MODULE_JSON_CONTENT)

    # 提取模块名称和版本
    string(JSON MODULE_NAME GET "${MODULE_JSON_CONTENT}" "name")
    string(JSON MODULE_VERSION GET "${MODULE_JSON_CONTENT}" "version")

    # 如果模块名称不是以 idcu- 或 idcu_ 开头，尝试根据目录结构确定
    get_filename_component(MODULE_PARENT_DIR "${IDCU_MODULE_MODULE_DIR}" DIRECTORY)
    get_filename_component(MODULE_GRANDPARENT_DIR "${MODULE_PARENT_DIR}" DIRECTORY)
    get_filename_component(MODULE_GRANDPARENT_NAME "${MODULE_GRANDPARENT_DIR}" NAME)
    get_filename_component(MODULE_PARENT_NAME "${MODULE_PARENT_DIR}" NAME)
    get_filename_component(MODULE_DIR_NAME "${IDCU_MODULE_MODULE_DIR}" NAME)

    # 检查是否需要根据目录结构生成模块名称
    set(ADJUSTED_MODULE_NAME "${MODULE_NAME}")
    if(NOT MODULE_NAME MATCHES "^idcu[-_]")
        if(MODULE_GRANDPARENT_NAME STREQUAL "modules")
            if(MODULE_PARENT_NAME STREQUAL "core")
                string(REPLACE "-" "_" MODULE_DIR_NAME_UNDERSCORE "${MODULE_DIR_NAME}")
                set(ADJUSTED_MODULE_NAME "idcu_core_${MODULE_DIR_NAME_UNDERSCORE}")
            elseif(MODULE_PARENT_NAME STREQUAL "business")
                string(REPLACE "-" "_" MODULE_DIR_NAME_UNDERSCORE "${MODULE_DIR_NAME}")
                set(ADJUSTED_MODULE_NAME "idcu_business_${MODULE_DIR_NAME_UNDERSCORE}")
            elseif(MODULE_PARENT_NAME STREQUAL "services")
                string(REPLACE "-" "_" MODULE_DIR_NAME_UNDERSCORE "${MODULE_DIR_NAME}")
                set(ADJUSTED_MODULE_NAME "idcu_services_${MODULE_DIR_NAME_UNDERSCORE}")
            elseif(MODULE_PARENT_NAME STREQUAL "integrations")
                string(REPLACE "-" "_" MODULE_DIR_NAME_UNDERSCORE "${MODULE_DIR_NAME}")
                set(ADJUSTED_MODULE_NAME "idcu_integrations_${MODULE_DIR_NAME_UNDERSCORE}")
            endif()
        endif()
    endif()

    # 调用通用核心函数，使用 idcu 命名空间
    build_module(
        MODULE_DIR "${IDCU_MODULE_MODULE_DIR}"
        NAMESPACE "idcu"
        INSTALL_NAMESPACE "idcu"
        ALIAS_PREFIXES "idcu" "idcu_core" "idcu_business" "idcu_services" "idcu_integrations"
    )

endfunction()

# 兼容函数：保持与旧版 API 一致
function(idcu_module_build)
    idcu_build_module(${ARGN})
endfunction()
