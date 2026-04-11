cmake_minimum_required(VERSION 3.15)
project(idcu-core-module C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-core-module STATIC
    src/idcu/core_module/core_module.c
)

target_include_directories(idcu-core-module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-core-module PUBLIC
    idcu::common
    idcu::log
    idcu::config
    idcu::healthcheck
    idcu::metrics
)

if(WIN32)
    target_link_libraries(idcu-core-module PRIVATE)
else()
    target_link_libraries(idcu-core-module PRIVATE pthread)
endif()

add_library(idcu::core_module ALIAS idcu-core-module)
