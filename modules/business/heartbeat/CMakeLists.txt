cmake_minimum_required(VERSION 3.15)
project(idcu-heartbeat C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-heartbeat STATIC
    src/idcu/heartbeat/heartbeat.c
)

target_include_directories(idcu-heartbeat PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-heartbeat PUBLIC
    idcu::common
    idcu::network
    idcu::metrics
    idcu::core_module
)

if(WIN32)
    target_link_libraries(idcu-heartbeat PRIVATE)
else()
    target_link_libraries(idcu-heartbeat PRIVATE pthread)
endif()

add_library(idcu::heartbeat ALIAS idcu-heartbeat)
