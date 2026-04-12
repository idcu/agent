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

option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_EXAMPLES "Build example programs" ON)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_MSAN "Enable MemorySanitizer" OFF)
option(ENABLE_CODE_COVERAGE "Enable code coverage instrumentation" OFF)
option(ENABLE_CPPCHECK "Enable cppcheck static analysis" OFF)

if(NOT MSVC)
    if(ENABLE_ASAN)
        message(STATUS "Enabling AddressSanitizer")
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()

    if(ENABLE_TSAN)
        message(STATUS "Enabling ThreadSanitizer")
        add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
        add_link_options(-fsanitize=thread)
    endif()

    if(ENABLE_UBSAN)
        message(STATUS "Enabling UndefinedBehaviorSanitizer")
        add_compile_options(-fsanitize=undefined -fno-sanitize-recover=all)
        add_link_options(-fsanitize=undefined)
    endif()

    if(ENABLE_MSAN)
        message(STATUS "Enabling MemorySanitizer")
        add_compile_options(-fsanitize=memory -fno-omit-frame-pointer)
        add_link_options(-fsanitize=memory)
    endif()
    
    if(ENABLE_CODE_COVERAGE)
        message(STATUS "Enabling code coverage instrumentation")
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/libs/idcu-module-build/cmake")

# Cppcheck static analysis configuration
if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXECUTABLE cppcheck)
    if(CPPCHECK_EXECUTABLE)
        message(STATUS "Found cppcheck: ${CPPCHECK_EXECUTABLE}")
        set(CMAKE_C_CPPCHECK "${CPPCHECK_EXECUTABLE}"
            "--enable=all"
            "--inconclusive"
            "--force"
            "--quiet"
            "--suppress=missingIncludeSystem"
            "--suppress=unmatchedSuppression"
            "--suppress=unusedFunction"
            "--std=c99"
        )
    else()
        message(WARNING "cppcheck not found, static analysis will not be available")
    endif()
endif()

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
message(STATUS "  Sanitizers:")
message(STATUS "    AddressSanitizer (ASAN): ${ENABLE_ASAN}")
message(STATUS "    ThreadSanitizer (TSAN): ${ENABLE_TSAN}")
message(STATUS "    UndefinedBehaviorSanitizer (UBSAN): ${ENABLE_UBSAN}")
message(STATUS "    MemorySanitizer (MSAN): ${ENABLE_MSAN}")
message(STATUS "  Code Coverage: ${ENABLE_CODE_COVERAGE}")
message(STATUS "  Cppcheck: ${ENABLE_CPPCHECK}")
message(STATUS "========================================")
message(STATUS "")
