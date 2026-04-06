# IDCU Module Build - 通用模块构建工具
# 基于 module.json 自动构建 C 语言模块

include(CMakeParseArguments)

function(idcu_build_module)
    cmake_parse_arguments(
        IDCU_MODULE
        ""
        "MODULE_DIR"
        ""
        ${ARGN}
    )

    if(NOT IDCU_MODULE_MODULE_DIR)
        set(IDCU_MODULE_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
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
    if(NOT MODULE_NAME MATCHES "^idcu[-_]")
        if(MODULE_GRANDPARENT_NAME STREQUAL "modules")
            if(MODULE_PARENT_NAME STREQUAL "core")
                string(REPLACE "-" "_" MODULE_DIR_NAME_UNDERSCORE "${MODULE_DIR_NAME}")
                set(MODULE_NAME "idcu_core_${MODULE_DIR_NAME_UNDERSCORE}")
            elseif(MODULE_PARENT_NAME STREQUAL "business")
                string(REPLACE "-" "_" MODULE_DIR_NAME_UNDERSCORE "${MODULE_DIR_NAME}")
                set(MODULE_NAME "idcu_business_${MODULE_DIR_NAME_UNDERSCORE}")
            elseif(MODULE_PARENT_NAME STREQUAL "services")
                string(REPLACE "-" "_" MODULE_DIR_NAME_UNDERSCORE "${MODULE_DIR_NAME}")
                set(MODULE_NAME "idcu_services_${MODULE_DIR_NAME_UNDERSCORE}")
            elseif(MODULE_PARENT_NAME STREQUAL "integrations")
                string(REPLACE "-" "_" MODULE_DIR_NAME_UNDERSCORE "${MODULE_DIR_NAME}")
                set(MODULE_NAME "idcu_integrations_${MODULE_DIR_NAME_UNDERSCORE}")
            endif()
        endif()
    endif()

    # 设置项目
    cmake_minimum_required(VERSION 3.10)
    project(${MODULE_NAME} VERSION ${MODULE_VERSION} LANGUAGES C)

    # 设置 C 标准
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_C_STANDARD_REQUIRED ON)

    # 检查是否有 build 配置
    string(JSON BUILD_CONFIG_ERROR ERROR_VARIABLE GET "${MODULE_JSON_CONTENT}" "build")
    set(HAS_BUILD_CONFIG OFF)
    set(BUILD_CONFIG "")
    if(NOT BUILD_CONFIG_ERROR)
        set(HAS_BUILD_CONFIG ON)
        string(JSON BUILD_CONFIG GET "${MODULE_JSON_CONTENT}" "build")
    endif()

    # 设置默认构建配置
    set(LANG_TYPE "C")
    set(LANG_STANDARD "99")
    set(LIB_TYPE "static")
    set(LINK_LIBRARIES "")
    set(PLATFORM_DEPS "")

    if(HAS_BUILD_CONFIG)
        # 解析 build 配置
        string(JSON LANG_TYPE_ERROR ERROR_VARIABLE GET "${BUILD_CONFIG}" "language")
        if(NOT LANG_TYPE_ERROR)
            string(JSON LANG_TYPE GET "${BUILD_CONFIG}" "language")
        endif()
        
        string(JSON LANG_STANDARD_ERROR ERROR_VARIABLE GET "${BUILD_CONFIG}" "standard")
        if(NOT LANG_STANDARD_ERROR)
            string(JSON LANG_STANDARD GET "${BUILD_CONFIG}" "standard")
        endif()
        
        string(JSON LIB_TYPE_ERROR ERROR_VARIABLE GET "${BUILD_CONFIG}" "type")
        if(NOT LIB_TYPE_ERROR)
            string(JSON LIB_TYPE GET "${BUILD_CONFIG}" "type")
        endif()
        
        # 解析 link_libraries
        string(JSON LINK_LIBS_ERROR ERROR_VARIABLE GET "${BUILD_CONFIG}" "link_libraries")
        if(NOT LINK_LIBS_ERROR)
            string(JSON LINK_LIBS GET "${BUILD_CONFIG}" "link_libraries")
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

    # 收集源文件
    file(GLOB_RECURSE SRCS
        "${IDCU_MODULE_MODULE_DIR}/src/*.c"
    )

    # 创建库
    if(LIB_TYPE STREQUAL "static")
        add_library(${MODULE_NAME} STATIC ${SRCS})
    elseif(LIB_TYPE STREQUAL "shared")
        add_library(${MODULE_NAME} SHARED ${SRCS})
    else()
        add_library(${MODULE_NAME} STATIC ${SRCS})
    endif()

    # 添加别名 - 支持多种格式
    set(MODULE_SHORT_NAMES "")
    
    # 格式 1: idcu-xxx -> idcu::xxx
    if(MODULE_NAME MATCHES "^idcu-")
        string(REGEX REPLACE "^idcu-" "" MODULE_SHORT_NAME "${MODULE_NAME}")
        list(APPEND MODULE_SHORT_NAMES "${MODULE_SHORT_NAME}")
    endif()
    
    # 格式 2: idcu_core_xxx -> idcu::core_xxx
    if(MODULE_NAME MATCHES "^idcu_core_")
        string(REGEX REPLACE "^idcu_core_" "" MODULE_SHORT_NAME "${MODULE_NAME}")
        list(APPEND MODULE_SHORT_NAMES "core_${MODULE_SHORT_NAME}")
        list(APPEND MODULE_SHORT_NAMES "${MODULE_SHORT_NAME}")
    endif()
    
    # 格式 3: idcu_business_xxx -> idcu::business_xxx
    if(MODULE_NAME MATCHES "^idcu_business_")
        string(REGEX REPLACE "^idcu_business_" "" MODULE_SHORT_NAME "${MODULE_NAME}")
        list(APPEND MODULE_SHORT_NAMES "business_${MODULE_SHORT_NAME}")
        list(APPEND MODULE_SHORT_NAMES "${MODULE_SHORT_NAME}")
    endif()
    
    # 格式 4: idcu_services_xxx -> idcu::services_xxx
    if(MODULE_NAME MATCHES "^idcu_services_")
        string(REGEX REPLACE "^idcu_services_" "" MODULE_SHORT_NAME "${MODULE_NAME}")
        list(APPEND MODULE_SHORT_NAMES "services_${MODULE_SHORT_NAME}")
        list(APPEND MODULE_SHORT_NAMES "${MODULE_SHORT_NAME}")
    endif()
    
    # 格式 5: idcu_integrations_xxx -> idcu::integrations_xxx
    if(MODULE_NAME MATCHES "^idcu_integrations_")
        string(REGEX REPLACE "^idcu_integrations_" "" MODULE_SHORT_NAME "${MODULE_NAME}")
        list(APPEND MODULE_SHORT_NAMES "integrations_${MODULE_SHORT_NAME}")
        list(APPEND MODULE_SHORT_NAMES "${MODULE_SHORT_NAME}")
    endif()
    
    # 添加所有可能的别名
    foreach(SHORT_NAME ${MODULE_SHORT_NAMES})
        if(NOT TARGET idcu::${SHORT_NAME})
            add_library(idcu::${SHORT_NAME} ALIAS ${MODULE_NAME})
        endif()
    endforeach()

    # 公共头文件目录
    if(HAS_BUILD_CONFIG)
        string(JSON INCLUDES_GET ERROR_VARIABLE INCLUDES_ERROR GET "${BUILD_CONFIG}" "includes")
        if(NOT INCLUDES_ERROR)
            string(JSON INCLUDES_COUNT LENGTH "${INCLUDES_GET}")
            if(INCLUDES_COUNT GREATER 0)
                math(EXPR INCLUDES_COUNT "${INCLUDES_COUNT} - 1")
                foreach(INDEX RANGE ${INCLUDES_COUNT})
                    string(JSON INCLUDE_PATH GET "${INCLUDES_GET}" ${INDEX})
                    target_include_directories(${MODULE_NAME} PUBLIC
                        $<BUILD_INTERFACE:${IDCU_MODULE_MODULE_DIR}/${INCLUDE_PATH}>
                    )
                endforeach()
            endif()
        else()
            # 默认只包含 include 目录
            target_include_directories(${MODULE_NAME} PUBLIC
                $<BUILD_INTERFACE:${IDCU_MODULE_MODULE_DIR}/include>
                $<INSTALL_INTERFACE:include>
            )
        endif()
    else()
        # 没有 build 配置时，默认只包含 include 目录
        target_include_directories(${MODULE_NAME} PUBLIC
            $<BUILD_INTERFACE:${IDCU_MODULE_MODULE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        )
    endif()

    # 链接依赖 - 从 link_libraries 获取
    foreach(DEP_NAME ${LINK_LIBRARIES})
        if(DEP_NAME MATCHES "^idcu-")
            string(REGEX REPLACE "^idcu-" "" DEP_SHORT "${DEP_NAME}")
            if(TARGET idcu::${DEP_SHORT})
                target_link_libraries(${MODULE_NAME} PUBLIC idcu::${DEP_SHORT})
            elseif(TARGET ${DEP_NAME})
                target_link_libraries(${MODULE_NAME} PUBLIC ${DEP_NAME})
            endif()
        elseif(TARGET idcu::${DEP_NAME})
            target_link_libraries(${MODULE_NAME} PUBLIC idcu::${DEP_NAME})
        elseif(TARGET ${DEP_NAME})
            target_link_libraries(${MODULE_NAME} PUBLIC ${DEP_NAME})
        endif()
    endforeach()

    # 处理平台依赖（仅在有 build 配置时）
    if(HAS_BUILD_CONFIG)
        string(JSON PLATFORM_DEPS_ERROR ERROR_VARIABLE GET "${BUILD_CONFIG}" "platform_deps")
        if(NOT PLATFORM_DEPS_ERROR)
            string(JSON PLATFORM_DEPS GET "${BUILD_CONFIG}" "platform_deps")
            if(WIN32)
                string(JSON WIN32_LIBS_ERROR ERROR_VARIABLE GET "${PLATFORM_DEPS}" "win32")
                if(NOT WIN32_LIBS_ERROR)
                    string(JSON WIN32_LIBS GET "${PLATFORM_DEPS}" "win32")
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
                string(JSON UNIX_LIBS_ERROR ERROR_VARIABLE GET "${PLATFORM_DEPS}" "unix")
                if(NOT UNIX_LIBS_ERROR)
                    string(JSON UNIX_LIBS GET "${PLATFORM_DEPS}" "unix")
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

    # 安装配置（仅在有 build 配置时）
    if(HAS_BUILD_CONFIG)
        include(GNUInstallDirs)
        include(CMakePackageConfigHelpers)

        # 安装库文件
        install(TARGETS ${MODULE_NAME}
            EXPORT ${MODULE_NAME}-targets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

        # 安装头文件
        if(EXISTS "${IDCU_MODULE_MODULE_DIR}/include")
            install(DIRECTORY "${IDCU_MODULE_MODULE_DIR}/include/"
                DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            )
        endif()

        # 导出目标
        install(EXPORT ${MODULE_NAME}-targets
            FILE ${MODULE_NAME}-targets.cmake
            NAMESPACE idcu::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${MODULE_NAME}
        )

        # 版本文件
        write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config-version.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY SameMajorVersion
        )

        # 生成配置文件
        set(CONFIG_CONTENT "include(CMakeFindDependencyMacro)\n")
        foreach(DEP_NAME ${LINK_LIBRARIES})
            string(APPEND CONFIG_CONTENT "find_dependency(${DEP_NAME})\n")
        endforeach()
        string(APPEND CONFIG_CONTENT "include(\"\${CMAKE_CURRENT_LIST_DIR}/${MODULE_NAME}-targets.cmake\")\n")

        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config.cmake" "${CONFIG_CONTENT}")

        # 安装配置文件
        install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config-version.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${MODULE_NAME}
        )
    endif()

    # 构建测试（仅在有 build 配置时）
    if(HAS_BUILD_CONFIG)
        if(NOT DEFINED BUILD_TESTS)
            option(BUILD_TESTS "Build tests" OFF)
        endif()
        if(BUILD_TESTS AND EXISTS "${IDCU_MODULE_MODULE_DIR}/tests")
            enable_testing()
            file(GLOB TEST_SRCS "${IDCU_MODULE_MODULE_DIR}/tests/test_*.c")
            foreach(test_src ${TEST_SRCS})
                get_filename_component(test_name ${test_src} NAME_WE)
                add_executable(${MODULE_NAME}_${test_name} ${test_src})
                if(MODULE_SHORT_NAMES)
                    list(GET MODULE_SHORT_NAMES 0 FIRST_SHORT_NAME)
                    target_link_libraries(${MODULE_NAME}_${test_name} PRIVATE idcu::${FIRST_SHORT_NAME})
                else()
                    target_link_libraries(${MODULE_NAME}_${test_name} PRIVATE ${MODULE_NAME})
                endif()
                add_test(NAME ${MODULE_NAME}_${test_name} COMMAND ${MODULE_NAME}_${test_name})
            endforeach()
        endif()

        # 构建示例（仅在有 build 配置时）
        if(NOT DEFINED BUILD_EXAMPLES)
            option(BUILD_EXAMPLES "Build examples" OFF)
        endif()
        if(BUILD_EXAMPLES AND EXISTS "${IDCU_MODULE_MODULE_DIR}/examples")
            file(GLOB EXAMPLE_SRCS "${IDCU_MODULE_MODULE_DIR}/examples/example_*.c")
            foreach(example_src ${EXAMPLE_SRCS})
                get_filename_component(example_name ${example_src} NAME_WE)
                add_executable(${MODULE_NAME}_${example_name} ${example_src})
                if(MODULE_SHORT_NAMES)
                    list(GET MODULE_SHORT_NAMES 0 FIRST_SHORT_NAME)
                    target_link_libraries(${MODULE_NAME}_${example_name} PRIVATE idcu::${FIRST_SHORT_NAME})
                else()
                    target_link_libraries(${MODULE_NAME}_${example_name} PRIVATE ${MODULE_NAME})
                endif()
            endforeach()
        endif()
    endif()

endfunction()
