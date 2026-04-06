cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(idcu-common VERSION 1.0.0 LANGUAGES C)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 收集源文件
file(GLOB_RECURSE SRCS
    src/*.c
)

# 创建静态库
add_library(idcu-common STATIC ${SRCS})
add_library(idcu::common ALIAS idcu-common)

# 公共头文件目录
target_include_directories(idcu-common PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Windows 平台需要链接必要的库
if(WIN32)
    target_link_libraries(idcu-common PUBLIC)
else()
    target_link_libraries(idcu-common PUBLIC pthread)
endif()

# 安装配置
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# 安装库文件
install(TARGETS idcu-common
    EXPORT idcu-common-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# 安装头文件
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# 导出目标
install(EXPORT idcu-common-targets
    FILE idcu-common-targets.cmake
    NAMESPACE idcu::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-common
)

# 配置文件
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/idcu-common-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-common-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-common
)

# 版本文件
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-common-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# 安装配置文件
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-common-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-common-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-common
)

# 构建测试
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    
    # 编译测试
    file(GLOB TEST_SRCS tests/test_*.c)
    foreach(test_src ${TEST_SRCS})
        get_filename_component(test_name ${test_src} NAME_WE)
        add_executable(${test_name} ${test_src})
        target_link_libraries(${test_name} PRIVATE idcu-common)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endif()

# 构建示例
option(BUILD_EXAMPLES "Build examples" ON)
if(BUILD_EXAMPLES)
    # 编译示例
    file(GLOB EXAMPLE_SRCS examples/example_*.c)
    foreach(example_src ${EXAMPLE_SRCS})
        get_filename_component(example_name ${example_src} NAME_WE)
        add_executable(${example_name} ${example_src})
        target_link_libraries(${example_name} PRIVATE idcu-common)
    endforeach()
endif()
