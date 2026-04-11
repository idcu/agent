cmake_minimum_required(VERSION 3.10)
project(idcu_tools)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find idcu packages
find_package(idcu-common REQUIRED)

# Memory Debugger Library
add_library(idcu-memory-debugger STATIC
    memory_debugger.c
)

target_include_directories(idcu-memory-debugger PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(idcu-memory-debugger
    idcu-common::idcu-common
)

# Performance Profiler Library
add_library(idcu-performance-profiler STATIC
    performance_profiler.c
)

target_include_directories(idcu-performance-profiler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(idcu-performance-profiler
    idcu-common::idcu-common
)

# Log Analyzer Library
add_library(idcu-log-analyzer STATIC
    log_analyzer.c
)

target_include_directories(idcu-log-analyzer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(idcu-log-analyzer
    idcu-common::idcu-common
)

# Module Inspector Library
add_library(idcu-module-inspector STATIC
    module_inspector.c
)

target_include_directories(idcu-module-inspector PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(idcu-module-inspector
    idcu-common::idcu-common
)
