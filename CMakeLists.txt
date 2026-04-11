cmake_minimum_required(VERSION 3.14)
project(idcu_agent C)

set(IDCU_AGENT_VERSION_MAJOR 0)
set(IDCU_AGENT_VERSION_MINOR 1)
set(IDCU_AGENT_VERSION_PATCH 0)
set(IDCU_AGENT_VERSION_STRING "${IDCU_AGENT_VERSION_MAJOR}.${IDCU_AGENT_VERSION_MINOR}.${IDCU_AGENT_VERSION_PATCH}")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(MSVC)
    add_compile_options(/W4 /utf-8)
    add_compile_options(/MP)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -pipe)
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
        add_compile_definitions(DEBUG=1)
    endif()
    
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3)
        add_compile_definitions(NDEBUG=1)
    endif()
endif()

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_EXAMPLES "Build example programs" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/libs/idcu-module-build/cmake")

add_subdirectory(libs)
add_subdirectory(modules)
add_subdirectory(app)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "")
message(STATUS "========================================")
message(STATUS "IDCU Agent Configuration Summary")
message(STATUS "========================================")
message(STATUS "  Version: ${IDCU_AGENT_VERSION_STRING}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "  Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "========================================")
message(STATUS "")
