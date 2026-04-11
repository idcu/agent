cmake_minimum_required(VERSION 3.15)
project(idcu-healthcheck-module C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-healthcheck-module STATIC
    src/idcu/healthcheck_module/healthcheck_module.c
)

target_include_directories(idcu-healthcheck-module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-healthcheck-module PUBLIC
    idcu::common
    idcu::healthcheck
    idcu::core_module
)

if(WIN32)
    target_link_libraries(idcu-healthcheck-module PRIVATE)
else()
    target_link_libraries(idcu-healthcheck-module PRIVATE pthread)
endif()

add_library(idcu::healthcheck_module ALIAS idcu-healthcheck-module)
