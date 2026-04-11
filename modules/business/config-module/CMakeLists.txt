cmake_minimum_required(VERSION 3.15)
project(idcu-config-module C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-config-module STATIC
    src/idcu/config_module/config_module.c
)

target_include_directories(idcu-config-module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-config-module PUBLIC
    idcu::common
    idcu::config
    idcu::yaml
    idcu::json
    idcu::msgbus
    idcu::core_module
)

if(WIN32)
    target_link_libraries(idcu-config-module PRIVATE)
else()
    target_link_libraries(idcu-config-module PRIVATE pthread)
endif()

add_library(idcu::config_module ALIAS idcu-config-module)
