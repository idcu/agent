cmake_minimum_required(VERSION 3.15)
project(idcu-module-build VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" OFF)

set(IDCU_MODULE_BUILD_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/config" CACHE PATH "Module build config directory")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/idcu/module_build/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/idcu/module_build/config.h"
    @ONLY
)

add_library(idcu-module-build INTERFACE)

target_include_directories(idcu-module-build INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_library(idcu::module-build ALIAS idcu-module-build)

function(idcu_add_module module_name)
    set(options STATIC SHARED MODULE)
    set(oneValueArgs VERSION DESCRIPTION)
    set(multiValueArgs SOURCES INCLUDE_DIRS LINK_LIBRARIES DEPENDS)
    cmake_parse_arguments(IDCU_MODULE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(IDCU_MODULE_STATIC)
        set(lib_type STATIC)
    elseif(IDCU_MODULE_SHARED)
        set(lib_type SHARED)
    elseif(IDCU_MODULE_MODULE)
        set(lib_type MODULE)
    else()
        set(lib_type STATIC)
    endif()

    add_library(${module_name} ${lib_type} ${IDCU_MODULE_SOURCES})

    if(IDCU_MODULE_VERSION)
        set_target_properties(${module_name} PROPERTIES VERSION ${IDCU_MODULE_VERSION})
    endif()

    if(IDCU_MODULE_DESCRIPTION)
        set_target_properties(${module_name} PROPERTIES DESCRIPTION ${IDCU_MODULE_DESCRIPTION})
    endif()

    if(IDCU_MODULE_INCLUDE_DIRS)
        target_include_directories(${module_name} PUBLIC ${IDCU_MODULE_INCLUDE_DIRS})
    endif()

    if(IDCU_MODULE_LINK_LIBRARIES)
        target_link_libraries(${module_name} PRIVATE ${IDCU_MODULE_LINK_LIBRARIES})
    endif()

    if(IDCU_MODULE_DEPENDS)
        add_dependencies(${module_name} ${IDCU_MODULE_DEPENDS})
    endif()
endfunction()

function(idcu_add_module_test test_name)
    if(NOT BUILD_TESTING)
        return()
    endif()

    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES LINK_LIBRARIES)
    cmake_parse_arguments(IDCU_TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${test_name} ${IDCU_TEST_SOURCES})
    target_link_libraries(${test_name} PRIVATE ${IDCU_TEST_LINK_LIBRARIES})
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
