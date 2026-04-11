cmake_minimum_required(VERSION 3.15)
project(idcu-network C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-network STATIC
    src/idcu/network/network.c
)

target_include_directories(idcu-network PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-network PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-network PRIVATE)
else()
    target_link_libraries(idcu-network PRIVATE pthread)
endif()

add_library(idcu::network ALIAS idcu-network)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
