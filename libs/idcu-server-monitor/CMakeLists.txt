cmake_minimum_required(VERSION 3.15)
project(idcu-server-monitor C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-server-monitor STATIC
    src/idcu/server_monitor/server_monitor.c
)

target_include_directories(idcu-server-monitor PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-server-monitor PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-server-monitor PRIVATE)
else()
    target_link_libraries(idcu-server-monitor PRIVATE pthread)
endif()

add_library(idcu::server-monitor ALIAS idcu-server-monitor)
add_library(idcu::server_monitor ALIAS idcu-server-monitor)
