cmake_minimum_required(VERSION 3.15)
project(idcu-distributed C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-distributed STATIC
    src/idcu/distributed/distributed.c
)

target_include_directories(idcu-distributed PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-distributed PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-distributed PRIVATE)
else()
    target_link_libraries(idcu-distributed PRIVATE pthread)
endif()

add_library(idcu::distributed ALIAS idcu-distributed)
