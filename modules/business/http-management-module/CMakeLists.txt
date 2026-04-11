cmake_minimum_required(VERSION 3.15)
project(idcu-http-management-module C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-http-management-module STATIC
    src/idcu/http_management_module/http_management_module.c
)

target_include_directories(idcu-http-management-module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-http-management-module PUBLIC
    idcu::common
    idcu::http_server
    idcu::management
    idcu::core_module
)

if(WIN32)
    target_link_libraries(idcu-http-management-module PRIVATE)
else()
    target_link_libraries(idcu-http-management-module PRIVATE pthread)
endif()

add_library(idcu::http_management_module ALIAS idcu-http-management-module)
