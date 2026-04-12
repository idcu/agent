include_guard()

find_package(Python3 COMPONENTS Interpreter)

function(idcu_add_library name)
    set(options "")
    set(oneValueArgs VERSION)
    set(multiValueArgs SOURCES HEADERS DEPENDS)

    cmake_parse_arguments(IDCU_LIB
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    add_library(${name} STATIC ${IDCU_LIB_SOURCES})

    target_include_directories(${name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )

    if(IDCU_LIB_DEPENDS)
        target_link_libraries(${name} PUBLIC ${IDCU_LIB_DEPENDS})
    endif()

    add_library(idcu::${name} ALIAS ${name})
endfunction()

function(idcu_add_module name)
    set(options "")
    set(oneValueArgs VERSION)
    set(multiValueArgs SOURCES DEPENDS)

    cmake_parse_arguments(IDCU_MODULE
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    add_library(${name} STATIC ${IDCU_MODULE_SOURCES})

    target_include_directories(${name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )

    if(IDCU_MODULE_DEPENDS)
        target_link_libraries(${name} PUBLIC ${IDCU_MODULE_DEPENDS})
    endif()
endfunction()

function(idcu_discover_modules RESULT_VAR ROOT_DIR)
    if(NOT Python3_Interpreter_FOUND)
        message(WARNING "Python3 not found, cannot discover modules via module.yaml")
        set(${RESULT_VAR} "" PARENT_SCOPE)
        return()
    endif()
    
    set(DISCOVER_SCRIPT "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../scripts/discover_modules.py")
    
    execute_process(
        COMMAND ${Python3_EXECUTABLE} ${DISCOVER_SCRIPT} ${ROOT_DIR}
        OUTPUT_VARIABLE MODULES_JSON
        RESULT_VARIABLE DISCOVER_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    if(DISCOVER_RESULT EQUAL 0)
        string(JSON MODULE_COUNT LENGTH ${MODULES_JSON})
        set(MODULES "")
        math(EXPR LAST_INDEX "${MODULE_COUNT} - 1")
        
        foreach(I RANGE 0 ${LAST_INDEX})
            string(JSON MODULE_PATH GET ${MODULES_JSON} ${I} "path")
            string(JSON MODULE_NAME GET ${MODULES_JSON} ${I} "name")
            list(APPEND MODULES "${MODULE_PATH}")
        endforeach()
        
        set(${RESULT_VAR} ${MODULES} PARENT_SCOPE)
    else()
        message(WARNING "Failed to discover modules via module.yaml")
        set(${RESULT_VAR} "" PARENT_SCOPE)
    endif()
endfunction()

function(idcu_load_module_config MODULE_PATH RESULT_VAR)
    if(NOT Python3_Interpreter_FOUND)
        set(${RESULT_VAR} "" PARENT_SCOPE)
        return()
    endif()
    
    set(LOAD_SCRIPT "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../scripts/load_module_config.py")
    
    execute_process(
        COMMAND ${Python3_EXECUTABLE} ${LOAD_SCRIPT} ${MODULE_PATH}
        OUTPUT_VARIABLE CONFIG_JSON
        RESULT_VARIABLE LOAD_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    if(LOAD_RESULT EQUAL 0)
        set(${RESULT_VAR} "${CONFIG_JSON}" PARENT_SCOPE)
    else()
        set(${RESULT_VAR} "" PARENT_SCOPE)
    endif()
endfunction()

function(idcu_add_library_from_yaml MODULE_PATH)
    idcu_load_module_config(${MODULE_PATH} CONFIG_JSON)
    
    if(NOT CONFIG_JSON)
        message(WARNING "Failed to load config for ${MODULE_PATH}, falling back to manual CMakeLists.txt")
        add_subdirectory(${MODULE_PATH})
        return()
    endif()
    
    string(JSON MODULE_NAME ERROR_VARIABLE ERR GET ${CONFIG_JSON} "name")
    if(ERR)
        message(WARNING "Invalid module.yaml in ${MODULE_PATH}, falling back to manual CMakeLists.txt")
        add_subdirectory(${MODULE_PATH})
        return()
    endif()
    
    string(JSON USE_LEGACY_BUILD ERROR_VARIABLE ERR GET ${CONFIG_JSON} "use_legacy_build")
    if(NOT ERR AND USE_LEGACY_BUILD)
        message(STATUS "Module ${MODULE_NAME} configured to use legacy build")
        add_subdirectory(${MODULE_PATH})
        return()
    endif()
    
    string(JSON MODULE_VERSION ERROR_VARIABLE ERR GET ${CONFIG_JSON} "version")
    if(ERR)
        set(MODULE_VERSION "0.0.0")
    endif()
    
    string(JSON MODULE_DESCRIPTION ERROR_VARIABLE ERR GET ${CONFIG_JSON} "description")
    if(ERR)
        set(MODULE_DESCRIPTION "")
    endif()
    
    message(STATUS "=" * 60)
    message(STATUS "加载模块配置: ${MODULE_NAME}")
    message(STATUS "  版本: ${MODULE_VERSION}")
    if(MODULE_DESCRIPTION)
        message(STATUS "  描述: ${MODULE_DESCRIPTION}")
    endif()
    
    string(JSON HAS_DEPENDS ERROR_VARIABLE ERR GET ${CONFIG_JSON} "dependencies")
    set(DEPENDS "")
    if(NOT ERR)
        string(JSON DEP_COUNT LENGTH ${CONFIG_JSON} "dependencies")
        if(DEP_COUNT GREATER 0)
            message(STATUS "  依赖:")
            math(EXPR LAST_INDEX "${DEP_COUNT} - 1")
            foreach(I RANGE 0 ${LAST_INDEX})
                string(JSON DEP GET ${CONFIG_JSON} "dependencies" ${I})
                message(STATUS "    - ${DEP}")
                string(REGEX REPLACE "^idcu-" "" DEP_ALIAS "${DEP}")
                list(APPEND DEPENDS "idcu::${DEP_ALIAS}")
            endforeach()
        endif()
    endif()
    
    string(JSON HAS_BUILD ERROR_VARIABLE ERR GET ${CONFIG_JSON} "build")
    if(NOT ERR)
        message(STATUS "  构建配置: 已配置")
    endif()
    
    string(JSON HAS_TESTING ERROR_VARIABLE ERR GET ${CONFIG_JSON} "testing")
    set(TESTING_ENABLED ON)
    if(NOT ERR)
        string(JSON TESTING_ENABLED ERROR_VARIABLE ERR GET ${CONFIG_JSON} "testing" "enabled")
        if(ERR)
            set(TESTING_ENABLED ON)
        endif()
    endif()
    message(STATUS "  测试: ${TESTING_ENABLED}")
    
    message(STATUS "=" * 60)
    message(STATUS "使用新构建系统构建: ${MODULE_NAME}")
    
    set(SOURCE_DIR "${MODULE_PATH}/src")
    set(SOURCES "")
    if(EXISTS ${SOURCE_DIR})
        file(GLOB_RECURSE SOURCES
            "${SOURCE_DIR}/*.c"
            "${SOURCE_DIR}/*.cpp"
        )
    endif()
    
    if(NOT SOURCES)
        message(STATUS "No sources found for ${MODULE_NAME}, falling back to manual CMakeLists.txt")
        add_subdirectory(${MODULE_PATH})
        return()
    endif()
    
    add_library(${MODULE_NAME} STATIC ${SOURCES})
    
    target_include_directories(${MODULE_NAME} PUBLIC
        $<BUILD_INTERFACE:${MODULE_PATH}/include>
        $<INSTALL_INTERFACE:include>
    )
    
    if(DEPENDS)
        target_link_libraries(${MODULE_NAME} PUBLIC ${DEPENDS})
    endif()
    
    string(JSON HAS_LINK_LIBRARIES ERROR_VARIABLE ERR GET ${CONFIG_JSON} "link_libraries")
    if(NOT ERR)
        string(JSON LINK_LIB_COUNT LENGTH ${CONFIG_JSON} "link_libraries")
        if(LINK_LIB_COUNT GREATER 0)
            message(STATUS "  链接库:")
            math(EXPR LAST_INDEX "${LINK_LIB_COUNT} - 1")
            foreach(I RANGE 0 ${LAST_INDEX})
                string(JSON LINK_LIB GET ${CONFIG_JSON} "link_libraries" ${I})
                message(STATUS "    - ${LINK_LIB}")
                target_link_libraries(${MODULE_NAME} PRIVATE ${LINK_LIB})
            endforeach()
        endif()
    endif()
    
    string(JSON HAS_PLATFORM_LINK ERROR_VARIABLE ERR GET ${CONFIG_JSON} "platform_link_libraries")
    if(NOT ERR)
        if(WIN32)
            string(JSON HAS_WIN32 ERROR_VARIABLE ERR GET ${CONFIG_JSON} "platform_link_libraries" "windows")
            if(NOT ERR)
                string(JSON WIN32_LIB_COUNT LENGTH ${CONFIG_JSON} "platform_link_libraries" "windows")
                if(WIN32_LIB_COUNT GREATER 0)
                    message(STATUS "  Windows平台链接库:")
                    math(EXPR LAST_INDEX "${WIN32_LIB_COUNT} - 1")
                    foreach(I RANGE 0 ${LAST_INDEX})
                        string(JSON WIN32_LIB GET ${CONFIG_JSON} "platform_link_libraries" "windows" ${I})
                        message(STATUS "    - ${WIN32_LIB}")
                        target_link_libraries(${MODULE_NAME} PRIVATE ${WIN32_LIB})
                    endforeach()
                endif()
            endif()
        else()
            string(JSON HAS_UNIX ERROR_VARIABLE ERR GET ${CONFIG_JSON} "platform_link_libraries" "unix")
            if(NOT ERR)
                string(JSON UNIX_LIB_COUNT LENGTH ${CONFIG_JSON} "platform_link_libraries" "unix")
                if(UNIX_LIB_COUNT GREATER 0)
                    message(STATUS "  Unix平台链接库:")
                    math(EXPR LAST_INDEX "${UNIX_LIB_COUNT} - 1")
                    foreach(I RANGE 0 ${LAST_INDEX})
                        string(JSON UNIX_LIB GET ${CONFIG_JSON} "platform_link_libraries" "unix" ${I})
                        message(STATUS "    - ${UNIX_LIB}")
                        target_link_libraries(${MODULE_NAME} PRIVATE ${UNIX_LIB})
                    endforeach()
                endif()
            endif()
        endif()
    endif()
    
    string(REGEX REPLACE "^idcu-" "" ALIAS_NAME "${MODULE_NAME}")
    add_library(idcu::${ALIAS_NAME} ALIAS ${MODULE_NAME})
    
    # 同时创建下划线格式的别名，兼容旧代码
    string(REPLACE "-" "_" ALIAS_NAME_UNDERSCORE "${ALIAS_NAME}")
    if(NOT ALIAS_NAME STREQUAL ALIAS_NAME_UNDERSCORE)
        add_library(idcu::${ALIAS_NAME_UNDERSCORE} ALIAS ${MODULE_NAME})
        message(STATUS "Added library: ${MODULE_NAME} (aliases: idcu::${ALIAS_NAME}, idcu::${ALIAS_NAME_UNDERSCORE})")
    else()
        message(STATUS "Added library: ${MODULE_NAME} (alias: idcu::${ALIAS_NAME})")
    endif()
    
    if(TESTING_ENABLED AND BUILD_TESTS)
        set(TEST_DIR "${MODULE_PATH}/tests")
        if(EXISTS ${TEST_DIR})
            file(GLOB TEST_SOURCES "${TEST_DIR}/*.c")
            if(TEST_SOURCES)
                foreach(TEST_SOURCE ${TEST_SOURCES})
                    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
                    add_executable(${TEST_NAME} ${TEST_SOURCE})
                    target_link_libraries(${TEST_NAME} PRIVATE ${MODULE_NAME} idcu::testframework)
                    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
                    message(STATUS "Added test: ${TEST_NAME}")
                endforeach()
            endif()
        endif()
    endif()
    
    message(STATUS "模块构建完成: ${MODULE_NAME}")
    message(STATUS "")
endfunction()

function(idcu_add_module_from_yaml MODULE_PATH)
    idcu_add_library_from_yaml(${MODULE_PATH})
endfunction()
