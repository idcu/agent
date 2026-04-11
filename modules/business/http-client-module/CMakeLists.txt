cmake_minimum_required(VERSION 3.15)
project(idcu-http-client-module C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-http-client-module STATIC
    src/idcu/http_client_module/http_client_module.c
)

target_include_directories(idcu-http-client-module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-http-client-module PUBLIC
    idcu::common
    idcu::http_client
    idcu::core_module
)

if(WIN32)
    target_link_libraries(idcu-http-client-module PRIVATE)
else()
    target_link_libraries(idcu-http-client-module PRIVATE pthread)
endif()

add_library(idcu::http_client_module ALIAS idcu-http-client-module)
