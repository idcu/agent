cmake_minimum_required(VERSION 3.15)
project(idcu-alert-module C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-alert-module STATIC
    src/idcu/alert_module/alert_module.c
)

target_include_directories(idcu-alert-module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-alert-module PUBLIC
    idcu::common
    idcu::alert
    idcu::core_module
)

if(WIN32)
    target_link_libraries(idcu-alert-module PRIVATE)
else()
    target_link_libraries(idcu-alert-module PRIVATE pthread)
endif()

add_library(idcu::alert_module ALIAS idcu-alert-module)
