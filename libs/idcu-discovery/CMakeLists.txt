cmake_minimum_required(VERSION 3.15)
project(idcu-discovery C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-discovery STATIC
    src/idcu/discovery/discovery.c
)

target_include_directories(idcu-discovery PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-discovery PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-discovery PRIVATE)
else()
    target_link_libraries(idcu-discovery PRIVATE pthread)
endif()

add_library(idcu::discovery ALIAS idcu-discovery)
