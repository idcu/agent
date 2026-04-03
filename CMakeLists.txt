cmake_minimum_required(VERSION 3.10)
project(idcu_agent C)
set(CMAKE_C_STANDARD 99)

set(IDCU_VERSION_MAJOR 1)
set(IDCU_VERSION_MINOR 0)
set(IDCU_VERSION_PATCH 0)
set(IDCU_VERSION "${IDCU_VERSION_MAJOR}.${IDCU_VERSION_MINOR}.${IDCU_VERSION_PATCH}")

if(MSVC)
    add_compile_options(/W4)
elseif(MINGW)
    add_compile_options(-O2 -Wall -Wextra)
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
    if(MSVC)
    elseif(MINGW)
    endif()
else()
    target_link_libraries(idcu_agent PRIVATE pthread dl)
    target_link_libraries(idcu_core PRIVATE pthread dl)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_link_options(idcu_agent PRIVATE -Wl,--whole-archive idcu_core -Wl,--no-whole-archive)
    endif()
endif()

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_BENCHMARKS "Build benchmarks" ON)
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)
option(ENABLE_CODE_COVERAGE "Enable code coverage with gcov/lcov" OFF)

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
        set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy not found, static analysis disabled")
    endif()
endif()

if(ENABLE_CODE_COVERAGE)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "Code coverage requires GCC or Clang compiler")
    endif()
    
    if(NOT UNIX)
        message(FATAL_ERROR "Code coverage is only supported on Unix-like systems (Linux/macOS)")
    endif()
    
    message(STATUS "Code coverage enabled")
    
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
    
    find_program(LCOV_EXE NAMES lcov)
    if(LCOV_EXE)
        message(STATUS "Found lcov: ${LCOV_EXE}")
    else()
        message(WARNING "lcov not found, HTML report generation disabled")
    endif()
    
    find_program(GENHTML_EXE NAMES genhtml)
    if(GENHTML_EXE)
        message(STATUS "Found genhtml: ${GENHTML_EXE}")
    else()
        message(WARNING "genhtml not found, HTML report generation disabled")
    endif()
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    
    if(ENABLE_CODE_COVERAGE)
        add_custom_target(coverage_clean
            COMMAND ${LCOV_EXE} --directory ${PROJECT_BINARY_DIR} --zerocounters
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
            COMMENT "Clearing coverage counters"
        )
        
        add_custom_target(coverage_capture
            COMMAND ${LCOV_EXE} --directory ${PROJECT_BINARY_DIR} --capture --output-file coverage.info
            COMMAND ${LCOV_EXE} --remove coverage.info '/usr/*' '*/tests/*' '*/modules/*' --output-file coverage_filtered.info
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
            COMMENT "Capturing coverage data"
            DEPENDS coverage_clean
        )
        
        if(GENHTML_EXE)
            add_custom_target(coverage_report
                COMMAND ${GENHTML_EXE} -o coverage_report coverage_filtered.info
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                COMMENT "Generating HTML coverage report"
                DEPENDS coverage_capture
            )
            
            add_custom_target(coverage
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                COMMAND ${CMAKE_COMMAND} --build . --target coverage_report
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                COMMENT "Running tests and generating coverage report"
                DEPENDS coverage_clean
            )
        else()
            add_custom_target(coverage
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                COMMAND ${CMAKE_COMMAND} --build . --target coverage_capture
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                COMMENT "Running tests and capturing coverage data"
                DEPENDS coverage_clean
            )
        endif()
    endif()
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

# 构建示例动态模块 (DLL/SO)
option(BUILD_SAMPLE_DLL "Build sample dynamic module" ON)
if(BUILD_SAMPLE_DLL)
    add_library(sample_module SHARED modules/dynamic/sample_module.c)
    set_target_properties(sample_module PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out
        PREFIX ""  # 在 Windows 上不添加 "lib" 前缀
    )
    # 为 Linux 平台设置符号可见性
    if(UNIX AND NOT APPLE)
        target_compile_options(sample_module PRIVATE -fvisibility=hidden)
    endif()
    target_link_libraries(sample_module PRIVATE idcu_core)
endif()

# 构建动态模块加载示例程序
option(BUILD_DYNAMIC_LOADER_EXAMPLE "Build dynamic loader example" ON)
if(BUILD_DYNAMIC_LOADER_EXAMPLE)
    add_executable(dynamic_module_example examples/dynamic_module_example.c)
    set_target_properties(dynamic_module_example PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out
    )
    target_link_libraries(dynamic_module_example PRIVATE idcu_core)
    if(NOT WIN32)
        target_link_libraries(dynamic_module_example PRIVATE pthread dl)
    endif()
endif()