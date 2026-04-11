cmake_minimum_required(VERSION 3.15)
project(idcu-scheduler VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-scheduler STATIC
    src/idcu/scheduler/scheduler.c
)

target_include_directories(idcu-scheduler PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-scheduler PRIVATE idcu::common)
add_library(idcu::scheduler ALIAS idcu-scheduler)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
