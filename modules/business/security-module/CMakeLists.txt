cmake_minimum_required(VERSION 3.15)
project(idcu-security-module C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-security-module STATIC
    src/idcu/security_module/security_module.c
)

target_include_directories(idcu-security-module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-security-module PUBLIC
    idcu::common
    idcu::sandbox
    idcu::permission
    idcu::core_module
)

if(WIN32)
    target_link_libraries(idcu-security-module PRIVATE)
else()
    target_link_libraries(idcu-security-module PRIVATE pthread)
endif()

add_library(idcu::security_module ALIAS idcu-security-module)
