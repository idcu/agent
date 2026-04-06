cmake_minimum_required(VERSION 3.10)

project(idcu-network VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRCS
    src/*.c
)

add_library(idcu-network STATIC ${SRCS})
add_library(idcu::network ALIAS idcu-network)

target_include_directories(idcu-network PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-network PUBLIC
    idcu::common
    idcu::log
)

if(WIN32)
    target_link_libraries(idcu-network PUBLIC ws2_32)
else()
    target_link_libraries(idcu-network PUBLIC pthread)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS idcu-network
    EXPORT idcu-network-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT idcu-network-targets
    FILE idcu-network-targets.cmake
    NAMESPACE idcu::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-network
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/idcu-network-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-network-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-network
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-network-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-network-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-network-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-network
)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    
    file(GLOB TEST_SRCS tests/test_*.c)
    foreach(test_src ${TEST_SRCS})
        get_filename_component(test_name ${test_src} NAME_WE)
        add_executable(${test_name} ${test_src})
        target_link_libraries(${test_name} PRIVATE idcu-network)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endif()

option(BUILD_EXAMPLES "Build examples" ON)
if(BUILD_EXAMPLES)
    file(GLOB EXAMPLE_SRCS examples/example_*.c)
    foreach(example_src ${EXAMPLE_SRCS})
        get_filename_component(example_name ${example_src} NAME_WE)
        add_executable(${example_name} ${example_src})
        target_link_libraries(${example_name} PRIVATE idcu-network)
    endforeach()
endif()
