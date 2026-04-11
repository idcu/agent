cmake_minimum_required(VERSION 3.15)
project(idcu-collect-module C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-collect-module STATIC
    src/idcu/collect_module/collect_module.c
)

target_include_directories(idcu-collect-module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-collect-module PUBLIC
    idcu::common
    idcu::device_collector
    idcu::network
    idcu::core_module
)

if(WIN32)
    target_link_libraries(idcu-collect-module PRIVATE)
else()
    target_link_libraries(idcu-collect-module PRIVATE pthread)
endif()

add_library(idcu::collect_module ALIAS idcu-collect-module)
