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

    # 提取模块名称
    string(JSON MODULE_NAME GET "${MODULE_JSON_CONTENT}" "name")
    string(JSON MODULE_VERSION GET "${MODULE_JSON_CONTENT}" "version")
    string(JSON MODULE_DESCRIPTION GET "${MODULE_JSON_CONTENT}" "description")

    # 设置项目
    cmake_minimum_required(VERSION 3.10)
    project(${MODULE_NAME} VERSION ${MODULE_VERSION} LANGUAGES C)

    # 设置 C 标准
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_C_STANDARD_REQUIRED ON)

    # 解析构建配置
    string(JSON BUILD_CONFIG GET "${MODULE_JSON_CONTENT}" "build")
    string(JSON LANG_TYPE GET "${BUILD_CONFIG}" "language")
    string(JSON LANG_STANDARD GET "${BUILD_CONFIG}" "standard")
    string(JSON LIB_TYPE GET "${BUILD_CONFIG}" "type")

    # 处理依赖关系
    string(JSON DEPENDENCIES GET "${MODULE_JSON_CONTENT}" "dependencies")
    string(JSON DEP_COUNT LENGTH "${DEPENDENCIES}")
    math(EXPR DEP_COUNT "${DEP_COUNT} - 1")

    set(MODULE_DEPENDENCIES "")
    foreach(DEP_INDEX RANGE ${DEP_COUNT})
        string(JSON DEP_NAME GET "${DEPENDENCIES}" ${DEP_INDEX})
        list(APPEND MODULE_DEPENDENCIES ${DEP_NAME})
        
        # 查找依赖包
        string(REPLACE "-" "_" DEP_VAR "${DEP_NAME}")
        string(TOUPPER "${DEP_VAR}" DEP_VAR_UPPER)
        if(NOT TARGET idcu::${DEP_NAME})
            find_package(${DEP_NAME} REQUIRED)
        endif()
    endforeach()

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

    # 添加别名
    string(REPLACE "-" "::" MODULE_ALIAS "idcu::${MODULE_NAME}")
    string(REPLACE "idcu-" "" MODULE_SHORT_NAME "${MODULE_NAME}")
    add_library(idcu::${MODULE_SHORT_NAME} ALIAS ${MODULE_NAME})

    # 公共头文件目录
    target_include_directories(${MODULE_NAME} PUBLIC
        $<BUILD_INTERFACE:${IDCU_MODULE_MODULE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )

    # 链接依赖
    foreach(DEP_NAME ${MODULE_DEPENDENCIES})
        string(REPLACE "-" "" DEP_SHORT "${DEP_NAME}")
        target_link_libraries(${MODULE_NAME} PUBLIC idcu::${DEP_SHORT})
    endforeach()

    # 处理平台依赖
    string(JSON PLATFORM_DEPS GET "${BUILD_CONFIG}" "platform_deps")
    if(WIN32)
        string(JSON WIN32_LIBS GET "${PLATFORM_DEPS}" "win32")
        string(JSON WIN32_LIB_COUNT LENGTH "${WIN32_LIBS}")
        if(WIN32_LIB_COUNT GREATER 0)
            math(EXPR WIN32_LIB_COUNT "${WIN32_LIB_COUNT} - 1")
            foreach(LIB_INDEX RANGE ${WIN32_LIB_COUNT})
                string(JSON LIB_NAME GET "${WIN32_LIBS}" ${LIB_INDEX})
                target_link_libraries(${MODULE_NAME} PUBLIC ${LIB_NAME})
            endforeach()
        endif()
    else()
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

    # 安装配置
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
    foreach(DEP_NAME ${MODULE_DEPENDENCIES})
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

    # 构建测试
    option(BUILD_TESTS "Build tests" OFF)
    if(BUILD_TESTS AND EXISTS "${IDCU_MODULE_MODULE_DIR}/tests")
        enable_testing()
        file(GLOB TEST_SRCS "${IDCU_MODULE_MODULE_DIR}/tests/test_*.c")
        foreach(test_src ${TEST_SRCS})
            get_filename_component(test_name ${test_src} NAME_WE)
            add_executable(${MODULE_NAME}_${test_name} ${test_src})
            target_link_libraries(${MODULE_NAME}_${test_name} PRIVATE idcu::${MODULE_SHORT_NAME})
            add_test(NAME ${MODULE_NAME}_${test_name} COMMAND ${MODULE_NAME}_${test_name})
        endforeach()
    endif()

    # 构建示例
    option(BUILD_EXAMPLES "Build examples" OFF)
    if(BUILD_EXAMPLES AND EXISTS "${IDCU_MODULE_MODULE_DIR}/examples")
        file(GLOB EXAMPLE_SRCS "${IDCU_MODULE_MODULE_DIR}/examples/example_*.c")
        foreach(example_src ${EXAMPLE_SRCS})
            get_filename_component(example_name ${example_src} NAME_WE)
            add_executable(${MODULE_NAME}_${example_name} ${example_src})
            target_link_libraries(${MODULE_NAME}_${example_name} PRIVATE idcu::${MODULE_SHORT_NAME})
        endforeach()
    endif()

endfunction()
