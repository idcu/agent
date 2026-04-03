cmake_minimum_required(VERSION 3.10)
project(idcu_agent C)
set(CMAKE_C_STANDARD 99)

set(IDCU_VERSION_MAJOR 1)
set(IDCU_VERSION_MINOR 0)
set(IDCU_VERSION_PATCH 0)
set(IDCU_VERSION "${IDCU_VERSION_MAJOR}.${IDCU_VERSION_MINOR}.${IDCU_VERSION_PATCH}")

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-O2 -Wall -Wextra)
endif()

include_directories(${PROJECT_SOURCE_DIR} include modules)

file(GLOB_RECURSE CORE_SRCS
    src/common/*.c
    src/utils/*.c
    src/scheduler/*.c
    src/module/*.c
    src/monitor/*.c
    src/network/*.c
    src/security/*.c
    src/plugin/*.c
    src/test/*.c
    src/kernel/*.c
)

file(GLOB_RECURSE MODULE_SRCS modules/*.c)

add_library(idcu_core STATIC ${CORE_SRCS} ${MODULE_SRCS})
set_target_properties(idcu_core PROPERTIES
    VERSION ${IDCU_VERSION}
    SOVERSION ${IDCU_VERSION_MAJOR}
    ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out
)

add_executable(idcu_agent main.c)
set_target_properties(idcu_agent PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out
)
target_link_libraries(idcu_agent PRIVATE idcu_core)

if(WIN32)
    target_link_libraries(idcu_agent PRIVATE)
else()
    target_link_libraries(idcu_agent PRIVATE pthread)
    target_link_libraries(idcu_core PRIVATE pthread)
endif()

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_BENCHMARKS "Build benchmarks" ON)
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
        set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy not found, static analysis disabled")
    endif()
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_BENCHMARKS)
    add_executable(idcu_benchmark
        benchmark.c
    )
    set_target_properties(idcu_benchmark PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out
    )
    target_link_libraries(idcu_benchmark PRIVATE idcu_core)
    if(NOT WIN32)
        target_link_libraries(idcu_benchmark PRIVATE pthread)
    endif()
endif()