cmake_minimum_required(VERSION 3.10)

project(idcu-alert VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT TARGET idcu::common)
    find_package(idcu-common REQUIRED)
endif()

if(NOT TARGET idcu::log)
    find_package(idcu-log REQUIRED)
endif()

if(NOT TARGET idcu::metrics)
    find_package(idcu-metrics REQUIRED)
endif()

file(GLOB_RECURSE SRCS
    src/*.c
)

add_library(idcu-alert STATIC ${SRCS})
add_library(idcu::alert ALIAS idcu-alert)

target_include_directories(idcu-alert PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-alert PUBLIC
    idcu::common
    idcu::log
    idcu::metrics
)

if(WIN32)
    target_link_libraries(idcu-alert PUBLIC ws2_32)
else()
    target_link_libraries(idcu-alert PUBLIC pthread)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS idcu-alert
    EXPORT idcu-alert-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT idcu-alert-targets
    FILE idcu-alert-targets.cmake
    NAMESPACE idcu::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-alert
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-alert-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/idcu-alert-config.cmake"
"include(CMakeFindDependencyMacro)
find_dependency(idcu-common)
find_dependency(idcu-log)
find_dependency(idcu-metrics)
include(\"\${CMAKE_CURRENT_LIST_DIR}/idcu-alert-targets.cmake\")
")

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-alert-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/idcu-alert-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/idcu-alert
)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    
    file(GLOB TEST_SRCS tests/test_*.c)
    foreach(test_src ${TEST_SRCS})
        get_filename_component(test_name ${test_src} NAME_WE)
        add_executable(${test_name} ${test_src})
        target_link_libraries(${test_name} PRIVATE idcu::alert)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endif()

option(BUILD_EXAMPLES "Build examples" ON)
if(BUILD_EXAMPLES)
    file(GLOB EXAMPLE_SRCS examples/example_*.c)
    foreach(example_src ${EXAMPLE_SRCS})
        get_filename_component(example_name ${example_src} NAME_WE)
        add_executable(${example_name} ${example_src})
        target_link_libraries(${example_name} PRIVATE idcu::alert)
    endforeach()
endif()
