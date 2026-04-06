cmake_minimum_required(VERSION 3.10)

project(idcu-module-build VERSION 1.0.0 LANGUAGES C)

message(STATUS "IDCU Module Build System v${PROJECT_VERSION}")

option(BUILD_EXAMPLES "Build example projects" ON)

if(BUILD_EXAMPLES)
    message(STATUS "Building examples")
    add_subdirectory(examples)
endif()

include(GNUInstallDirs)

install(FILES
    cmake/idcu_module_build.cmake
    cmake/core/module_build_core.cmake
    cmake/adapters/idcu_agent_adapter.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-module-build
)

install(FILES
    config/default.yaml
    config/auto-adapter.yaml
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-module-build/config
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-module-build-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/idcu-module-build-config.cmake"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/idcu_module_build.cmake\")"
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-module-build-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-module-build-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-module-build
)
