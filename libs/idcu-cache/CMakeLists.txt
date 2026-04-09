cmake_minimum_required(VERSION 3.15)
project(idcu-cache C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-cache STATIC
    src/idcu/cache/cache.c
)

target_include_directories(idcu-cache PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-cache PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-cache PRIVATE)
else()
    target_link_libraries(idcu-cache PRIVATE pthread)
endif()

add_library(idcu::cache ALIAS idcu-cache)
