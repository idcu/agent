cmake_minimum_required(VERSION 3.15)
project(idcu-metrics-module C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-metrics-module STATIC
    src/idcu/metrics_module/metrics_module.c
)

target_include_directories(idcu-metrics-module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-metrics-module PUBLIC
    idcu::common
    idcu::metrics
    idcu::alert
    idcu::core_module
)

if(WIN32)
    target_link_libraries(idcu-metrics-module PRIVATE)
else()
    target_link_libraries(idcu-metrics-module PRIVATE pthread)
endif()

add_library(idcu::metrics_module ALIAS idcu-metrics-module)
