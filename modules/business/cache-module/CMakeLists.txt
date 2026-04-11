cmake_minimum_required(VERSION 3.15)
project(idcu-cache-module C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-cache-module STATIC
    src/idcu/cache_module/cache_module.c
)

target_include_directories(idcu-cache-module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-cache-module PUBLIC
    idcu::common
    idcu::cache
    idcu::core_module
)

if(WIN32)
    target_link_libraries(idcu-cache-module PRIVATE)
else()
    target_link_libraries(idcu-cache-module PRIVATE pthread)
endif()

add_library(idcu::cache_module ALIAS idcu-cache-module)
