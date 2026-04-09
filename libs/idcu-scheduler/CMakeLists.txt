cmake_minimum_required(VERSION 3.15)
project(idcu-scheduler C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-scheduler STATIC
    src/idcu/scheduler/scheduler.c
)

target_include_directories(idcu-scheduler PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-scheduler PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-scheduler PRIVATE)
else()
    target_link_libraries(idcu-scheduler PRIVATE pthread)
endif()

add_library(idcu::scheduler ALIAS idcu-scheduler)
