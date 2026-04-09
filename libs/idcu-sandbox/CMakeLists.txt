cmake_minimum_required(VERSION 3.15)
project(idcu-sandbox C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-sandbox STATIC
    src/idcu/sandbox/sandbox.c
)

target_include_directories(idcu-sandbox PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-sandbox PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-sandbox PRIVATE)
else()
    target_link_libraries(idcu-sandbox PRIVATE pthread)
endif()

add_library(idcu::sandbox ALIAS idcu-sandbox)
