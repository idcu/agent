cmake_minimum_required(VERSION 3.10)
project(idcu_agent C)
set(CMAKE_C_STANDARD 99)
add_compile_options(-O2 -Wall -Wextra -fno-stack-protector -ffunction-sections -fdata-sections)

include_directories(include kernel modules)

file(GLOB_RECURSE SRCS
    main.c
    modules/*.c
    kernel/*.c
)

add_executable(idcu_agent ${SRCS})
set_target_properties(idcu_agent PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out
)