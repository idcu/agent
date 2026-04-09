cmake_minimum_required(VERSION 3.15)
project(idcu-management C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-management STATIC
    src/idcu/management/management.c
)

target_include_directories(idcu-management PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-management PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-management PRIVATE)
else()
    target_link_libraries(idcu-management PRIVATE pthread)
endif()

add_library(idcu::management ALIAS idcu-management)
