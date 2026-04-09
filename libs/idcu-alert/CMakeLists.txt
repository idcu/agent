cmake_minimum_required(VERSION 3.15)
project(idcu-alert C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-alert STATIC
    src/idcu/alert/alert.c
)

target_include_directories(idcu-alert PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-alert PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-alert PRIVATE)
else()
    target_link_libraries(idcu-alert PRIVATE pthread)
endif()

add_library(idcu::alert ALIAS idcu-alert)
