cmake_minimum_required(VERSION 3.10)

project(idcu-module-build VERSION 1.0.0 LANGUAGES C)

# 安装 CMake 模块到系统
include(GNUInstallDirs)

install(FILES
    cmake/idcu_module_build.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-module-build
)

# 配置文件
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-module-build-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/idcu-module-build-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-module-build-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-module-build
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-module-build-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-module-build-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-module-build
)
