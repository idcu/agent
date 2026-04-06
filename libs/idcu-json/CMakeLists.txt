cmake_minimum_required(VERSION 3.10)

project(idcu-json VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT TARGET idcu::common)
    find_package(idcu-common REQUIRED)
endif()

file(GLOB_RECURSE SRCS
    src/*.c
)

add_library(idcu-json STATIC ${SRCS})
add_library(idcu::json ALIAS idcu-json)

target_include_directories(idcu-json PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-json PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-json PUBLIC)
else()
    target_link_libraries(idcu-json PUBLIC pthread)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS idcu-json
    EXPORT idcu-json-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT idcu-json-targets
    FILE idcu-json-targets.cmake
    NAMESPACE idcu::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-json
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/idcu-json-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-json-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-json
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-json-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-json-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-json-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-json
)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    
    file(GLOB TEST_SRCS tests/test_*.c)
    foreach(test_src ${TEST_SRCS})
        get_filename_component(test_name ${test_src} NAME_WE)
        add_executable(${test_name} ${test_src})
        target_link_libraries(${test_name} PRIVATE idcu::json)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endif()

option(BUILD_EXAMPLES "Build examples" ON)
if(BUILD_EXAMPLES)
    file(GLOB EXAMPLE_SRCS examples/example_*.c)
    foreach(example_src ${EXAMPLE_SRCS})
        get_filename_component(example_name ${example_src} NAME_WE)
        add_executable(${example_name} ${example_src})
        target_link_libraries(${example_name} PRIVATE idcu::json)
    endforeach()
endif()
