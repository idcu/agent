cmake_minimum_required(VERSION 3.10)
project(idcu_benchmarks)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find idcu packages
find_package(idcu-common REQUIRED)
find_package(idcu-msgbus REQUIRED)

# Benchmark: Common Library
add_executable(benchmark_common benchmark_common.c)
target_link_libraries(benchmark_common
    idcu-common::idcu-common
)
