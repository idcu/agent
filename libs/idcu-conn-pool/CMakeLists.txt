cmake_minimum_required(VERSION 3.15)
project(idcu-conn-pool C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-conn-pool STATIC
    src/idcu/conn_pool/conn_pool.c
)

target_include_directories(idcu-conn-pool PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-conn-pool PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-conn-pool PRIVATE)
else()
    target_link_libraries(idcu-conn-pool PRIVATE pthread)
endif()

add_library(idcu::conn-pool ALIAS idcu-conn-pool)
add_library(idcu::conn_pool ALIAS idcu-conn-pool)
