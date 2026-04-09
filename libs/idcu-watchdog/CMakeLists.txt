cmake_minimum_required(VERSION 3.15)
project(idcu-watchdog C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-watchdog STATIC
    src/idcu/watchdog/watchdog.c
)

target_include_directories(idcu-watchdog PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-watchdog PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-watchdog PRIVATE)
else()
    target_link_libraries(idcu-watchdog PRIVATE pthread)
endif()

add_library(idcu::watchdog ALIAS idcu-watchdog)
