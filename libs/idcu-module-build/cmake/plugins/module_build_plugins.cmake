# Module Build Plugins - 插件系统核心
# 提供插件注册、加载和执行机制

include(CMakeParseArguments)

# 保存插件系统的目录路径
set(MODULE_BUILD_PLUGINS_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "Module build plugins directory")

# 全局插件注册表
set(REGISTERED_PLUGINS "" CACHE INTERNAL "Registered plugins list")
set(PLUGIN_ENABLED_MAP "" CACHE INTERNAL "Map of plugin enabled status")

# ==============================================================================
# 插件注册机制
# ==============================================================================

# 注册插件
function(register_plugin)
    cmake_parse_arguments(
        REGISTER_PLUGIN
        ""
        "NAME;DESCRIPTION;VERSION;AUTHOR"
        "HOOKS;DEPENDS"
        ${ARGN}
    )

    if(NOT REGISTER_PLUGIN_NAME)
        message(FATAL_ERROR "Plugin NAME is required")
    endif()

    # 检查插件是否已注册
    if("${REGISTER_PLUGIN_NAME}" IN_LIST REGISTERED_PLUGINS)
        message(WARNING "Plugin ${REGISTER_PLUGIN_NAME} already registered, skipping")
        return()
    endif()

    # 存储插件信息
    set(PLUGIN_${REGISTER_PLUGIN_NAME}_NAME "${REGISTER_PLUGIN_NAME}" CACHE INTERNAL "Plugin name")
    set(PLUGIN_${REGISTER_PLUGIN_NAME}_DESCRIPTION "${REGISTER_PLUGIN_DESCRIPTION}" CACHE INTERNAL "Plugin description")
    set(PLUGIN_${REGISTER_PLUGIN_NAME}_VERSION "${REGISTER_PLUGIN_VERSION}" CACHE INTERNAL "Plugin version")
    set(PLUGIN_${REGISTER_PLUGIN_NAME}_AUTHOR "${REGISTER_PLUGIN_AUTHOR}" CACHE INTERNAL "Plugin author")
    set(PLUGIN_${REGISTER_PLUGIN_NAME}_HOOKS "${REGISTER_PLUGIN_HOOKS}" CACHE INTERNAL "Plugin hooks")
    set(PLUGIN_${REGISTER_PLUGIN_NAME}_DEPENDS "${REGISTER_PLUGIN_DEPENDS}" CACHE INTERNAL "Plugin dependencies")
    set(PLUGIN_${REGISTER_PLUGIN_NAME}_ENABLED OFF CACHE INTERNAL "Plugin enabled status")

    # 添加到注册表
    list(APPEND REGISTERED_PLUGINS "${REGISTER_PLUGIN_NAME}")
    set(REGISTERED_PLUGINS "${REGISTERED_PLUGINS}" CACHE INTERNAL "Registered plugins list")

    message(STATUS "Registered plugin: ${REGISTER_PLUGIN_NAME} v${REGISTER_PLUGIN_VERSION}")
endfunction()

# ==============================================================================
# 插件启用/禁用机制
# ==============================================================================

# 启用插件
function(enable_plugin PLUGIN_NAME)
    if(NOT "${PLUGIN_NAME}" IN_LIST REGISTERED_PLUGINS)
        message(WARNING "Cannot enable unknown plugin: ${PLUGIN_NAME}")
        return()
    endif()

    # 检查依赖
    get_plugin_dependencies("${PLUGIN_NAME}" DEPENDS)
    foreach(DEP ${DEPENDS})
        if(NOT PLUGIN_${DEP}_ENABLED)
            message(STATUS "Enabling dependency plugin: ${DEP}")
            enable_plugin("${DEP}")
        endif()
    endforeach()

    set(PLUGIN_${PLUGIN_NAME}_ENABLED ON CACHE INTERNAL "Plugin enabled status")
    message(STATUS "Enabled plugin: ${PLUGIN_NAME}")
endfunction()

# 禁用插件
function(disable_plugin PLUGIN_NAME)
    if(NOT "${PLUGIN_NAME}" IN_LIST REGISTERED_PLUGINS)
        message(WARNING "Cannot disable unknown plugin: ${PLUGIN_NAME}")
        return()
    endif()

    set(PLUGIN_${PLUGIN_NAME}_ENABLED OFF CACHE INTERNAL "Plugin enabled status")
    message(STATUS "Disabled plugin: ${PLUGIN_NAME}")
endfunction()

# 检查插件是否启用
function(is_plugin_enabled PLUGIN_NAME RESULT_VAR)
    if("${PLUGIN_NAME}" IN_LIST REGISTERED_PLUGINS AND PLUGIN_${PLUGIN_NAME}_ENABLED)
        set(${RESULT_VAR} ON PARENT_SCOPE)
    else()
        set(${RESULT_VAR} OFF PARENT_SCOPE)
    endif()
endfunction()

# ==============================================================================
# 插件信息查询
# ==============================================================================

# 获取插件描述
function(get_plugin_description PLUGIN_NAME RESULT_VAR)
    set(${RESULT_VAR} "${PLUGIN_${PLUGIN_NAME}_DESCRIPTION}" PARENT_SCOPE)
endfunction()

# 获取插件版本
function(get_plugin_version PLUGIN_NAME RESULT_VAR)
    set(${RESULT_VAR} "${PLUGIN_${PLUGIN_NAME}_VERSION}" PARENT_SCOPE)
endfunction()

# 获取插件钩子
function(get_plugin_hooks PLUGIN_NAME RESULT_VAR)
    set(${RESULT_VAR} "${PLUGIN_${PLUGIN_NAME}_HOOKS}" PARENT_SCOPE)
endfunction()

# 获取插件依赖
function(get_plugin_dependencies PLUGIN_NAME RESULT_VAR)
    set(${RESULT_VAR} "${PLUGIN_${PLUGIN_NAME}_DEPENDS}" PARENT_SCOPE)
endfunction()

# 获取所有已注册插件
function(get_registered_plugins RESULT_VAR)
    set(${RESULT_VAR} "${REGISTERED_PLUGINS}" PARENT_SCOPE)
endfunction()

# 获取所有已启用插件
function(get_enabled_plugins RESULT_VAR)
    set(ENABLED_PLUGINS "")
    foreach(PLUGIN ${REGISTERED_PLUGINS})
        if(PLUGIN_${PLUGIN}_ENABLED)
            list(APPEND ENABLED_PLUGINS "${PLUGIN}")
        endif()
    endforeach()
    set(${RESULT_VAR} "${ENABLED_PLUGINS}" PARENT_SCOPE)
endfunction()

# ==============================================================================
# 钩子执行机制
# ==============================================================================

# 定义钩子类型
set(PLUGIN_HOOKS
    "pre_build"
    "post_configure"
    "post_build"
    "pre_package"
    "post_package"
    "pre_publish"
    "post_publish"
)

# 执行钩子
function(execute_hook)
    cmake_parse_arguments(
        EXECUTE_HOOK
        ""
        "HOOK_NAME;MODULE_DIR;MODULE_NAME;MODULE_VERSION"
        ""
        ${ARGN}
    )

    if(NOT EXECUTE_HOOK_HOOK_NAME)
        message(FATAL_ERROR "HOOK_NAME is required for execute_hook")
    endif()

    message(STATUS "Executing hook: ${EXECUTE_HOOK_HOOK_NAME}")

    # 遍历所有已启用的插件
    foreach(PLUGIN ${REGISTERED_PLUGINS})
        if(NOT PLUGIN_${PLUGIN}_ENABLED)
            continue()
        endif()

        # 检查插件是否注册了该钩子
        get_plugin_hooks("${PLUGIN}" PLUGIN_HOOKS)
        if(NOT "${EXECUTE_HOOK_HOOK_NAME}" IN_LIST PLUGIN_HOOKS)
            continue()
        endif()

        # 执行插件的钩子函数 - 使用硬编码处理避免动态函数调用问题
        message(STATUS "  Running plugin: ${PLUGIN}")
        
        if(PLUGIN STREQUAL "version_generator")
            if(EXECUTE_HOOK_HOOK_NAME STREQUAL "pre_build")
                version_generator_pre_build(
                    MODULE_DIR "${EXECUTE_HOOK_MODULE_DIR}"
                    MODULE_NAME "${EXECUTE_HOOK_MODULE_NAME}"
                    MODULE_VERSION "${EXECUTE_HOOK_MODULE_VERSION}"
                )
            endif()
        elseif(PLUGIN STREQUAL "signer")
            if(EXECUTE_HOOK_HOOK_NAME STREQUAL "post_package")
                signer_post_package(
                    MODULE_DIR "${EXECUTE_HOOK_MODULE_DIR}"
                    MODULE_NAME "${EXECUTE_HOOK_MODULE_NAME}"
                    MODULE_VERSION "${EXECUTE_HOOK_MODULE_VERSION}"
                )
            endif()
        elseif(PLUGIN STREQUAL "coverage")
            if(EXECUTE_HOOK_HOOK_NAME STREQUAL "pre_build")
                coverage_pre_build(
                    MODULE_DIR "${EXECUTE_HOOK_MODULE_DIR}"
                    MODULE_NAME "${EXECUTE_HOOK_MODULE_NAME}"
                    MODULE_VERSION "${EXECUTE_HOOK_MODULE_VERSION}"
                )
            elseif(EXECUTE_HOOK_HOOK_NAME STREQUAL "post_build")
                coverage_post_build(
                    MODULE_DIR "${EXECUTE_HOOK_MODULE_DIR}"
                    MODULE_NAME "${EXECUTE_HOOK_MODULE_NAME}"
                    MODULE_VERSION "${EXECUTE_HOOK_MODULE_VERSION}"
                )
            endif()
        elseif(PLUGIN STREQUAL "packager")
            if(EXECUTE_HOOK_HOOK_NAME STREQUAL "post_package")
                packager_post_package(
                    MODULE_DIR "${EXECUTE_HOOK_MODULE_DIR}"
                    MODULE_NAME "${EXECUTE_HOOK_MODULE_NAME}"
                    MODULE_VERSION "${EXECUTE_HOOK_MODULE_VERSION}"
                )
            endif()
        elseif(PLUGIN STREQUAL "publisher")
            if(EXECUTE_HOOK_HOOK_NAME STREQUAL "post_package")
                publisher_post_package(
                    MODULE_DIR "${EXECUTE_HOOK_MODULE_DIR}"
                    MODULE_NAME "${EXECUTE_HOOK_MODULE_NAME}"
                    MODULE_VERSION "${EXECUTE_HOOK_MODULE_VERSION}"
                )
            endif()
        else()
            message(WARNING "Unknown plugin: ${PLUGIN}")
        endif()
    endforeach()
endfunction()

# ==============================================================================
# 从 module.json 加载插件配置
# ==============================================================================

# 从 module.json 加载并配置插件
function(load_plugins_from_config)
    cmake_parse_arguments(
        LOAD_PLUGINS
        ""
        "MODULE_DIR"
        ""
        ${ARGN}
    )

    if(NOT LOAD_PLUGINS_MODULE_DIR)
        set(LOAD_PLUGINS_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    set(MODULE_JSON_PATH "${LOAD_PLUGINS_MODULE_DIR}/module.json")
    if(NOT EXISTS "${MODULE_JSON_PATH}")
        message(STATUS "No module.json found, skipping plugin config loading")
        return()
    endif()

    file(READ "${MODULE_JSON_PATH}" MODULE_JSON_CONTENT)

    # 检查是否有 plugins 配置
    string(JSON PLUGINS_CONFIG GET "${MODULE_JSON_CONTENT}" "plugins")
    if(NOT PLUGINS_CONFIG)
        message(STATUS "No plugins configuration found in module.json")
        return()
    endif()

    message(STATUS "Loading plugins from configuration...")

    # 硬编码检查已知的插件
    foreach(PLUGIN_NAME "version_generator" "signer" "coverage" "packager" "publisher")
        string(JSON PLUGIN_VALUE GET "${PLUGINS_CONFIG}" "${PLUGIN_NAME}")
        if(PLUGIN_VALUE)
            enable_plugin("${PLUGIN_NAME}")
        else()
            disable_plugin("${PLUGIN_NAME}")
        endif()
    endforeach()

    get_enabled_plugins(ENABLED_PLUGINS)
    list(LENGTH ENABLED_PLUGINS ENABLED_COUNT)
    message(STATUS "Enabled ${ENABLED_COUNT} plugins: ${ENABLED_PLUGINS}")
endfunction()

# ==============================================================================
# 初始化插件系统
# ==============================================================================

# 初始化插件系统
function(init_plugins)
    message(STATUS "========================================")
    message(STATUS "Initializing Plugin System")
    message(STATUS "========================================")

    # 加载所有内置插件
    set(PLUGINS_DIR "${MODULE_BUILD_PLUGINS_DIR}")
    message(STATUS "Plugins directory: ${PLUGINS_DIR}")

    # 列出所有插件文件
    file(GLOB PLUGIN_FILES "${PLUGINS_DIR}/plugin_*.cmake")
    message(STATUS "Found ${PLUGIN_FILES} plugin files")

    foreach(PLUGIN_FILE ${PLUGIN_FILES})
        get_filename_component(PLUGIN_FILENAME "${PLUGIN_FILE}" NAME)
        message(STATUS "Loading plugin: ${PLUGIN_FILENAME}")
        include("${PLUGIN_FILE}")
    endforeach()

    get_registered_plugins(REGISTERED_LIST)
    list(LENGTH REGISTERED_LIST REGISTERED_COUNT)
    message(STATUS "Registered ${REGISTERED_COUNT} plugins total")
    message(STATUS "========================================")
endfunction()
