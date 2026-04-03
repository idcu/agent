cmake_minimum_required(VERSION 3.10)
project(idcu_agent C)
set(CMAKE_C_STANDARD 99)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-O2 -Wall -Wextra)
endif()

include_directories(${PROJECT_SOURCE_DIR} include modules)

file(GLOB_RECURSE SRCS
    main.c
    modules/*.c
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

add_executable(idcu_agent ${SRCS})
set_target_properties(idcu_agent PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out
)

if(WIN32)
    target_link_libraries(idcu_agent PRIVATE)
else()
    target_link_libraries(idcu_agent PRIVATE pthread)
endif()

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_BENCHMARKS)
    file(GLOB_RECURSE KERNEL_SRCS 
        "src/common/*.c"
        "src/utils/*.c"
        "src/scheduler/*.c"
        "src/module/*.c"
        "src/monitor/*.c"
        "src/network/*.c"
        "src/security/*.c"
        "src/plugin/*.c"
        "src/test/*.c"
    )
    list(FILTER KERNEL_SRCS EXCLUDE REGEX "micro_kernel.c")
    add_executable(idcu_benchmark
        benchmark.c
        ${KERNEL_SRCS}
    )
    set_target_properties(idcu_benchmark PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out
    )
    if(NOT WIN32)
        target_link_libraries(idcu_benchmark PRIVATE pthread)
    endif()
endif()