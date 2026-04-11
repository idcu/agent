cmake_minimum_required(VERSION 3.15)
project(idcu-config C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-config STATIC
    src/idcu/config/config.c
)

target_include_directories(idcu-config PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-config PUBLIC idcu::common idcu::json idcu::yaml)

if(WIN32)
    target_link_libraries(idcu-config PRIVATE)
else()
    target_link_libraries(idcu-config PRIVATE pthread)
endif()

add_library(idcu::config ALIAS idcu-config)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
