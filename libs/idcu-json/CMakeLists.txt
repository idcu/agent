cmake_minimum_required(VERSION 3.15)
project(idcu-json C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-json STATIC
    src/idcu/json/json.c
)

target_include_directories(idcu-json PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-json PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-json PRIVATE)
else()
    target_link_libraries(idcu-json PRIVATE pthread)
endif()

add_library(idcu::json ALIAS idcu-json)
