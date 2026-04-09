cmake_minimum_required(VERSION 3.15)
project(idcu-permission C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-permission STATIC
    src/idcu/permission/permission.c
)

target_include_directories(idcu-permission PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-permission PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-permission PRIVATE)
else()
    target_link_libraries(idcu-permission PRIVATE pthread)
endif()

add_library(idcu::permission ALIAS idcu-permission)
