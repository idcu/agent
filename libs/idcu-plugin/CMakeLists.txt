cmake_minimum_required(VERSION 3.15)
project(idcu-plugin C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-plugin STATIC
    src/idcu/plugin/plugin.c
)

target_include_directories(idcu-plugin PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-plugin PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-plugin PRIVATE)
else()
    target_link_libraries(idcu-plugin PRIVATE pthread)
endif()

add_library(idcu::plugin ALIAS idcu-plugin)
