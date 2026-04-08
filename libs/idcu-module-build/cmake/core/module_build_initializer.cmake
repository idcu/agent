# 初始化模块
# 负责初始化构建系统

include("${CMAKE_CURRENT_LIST_DIR}/platform_detection.cmake")

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
