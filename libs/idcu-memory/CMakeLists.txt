cmake_minimum_required(VERSION 3.15)
project(idcu-memory C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-memory STATIC
    src/idcu/memory/memory.c
)

target_include_directories(idcu-memory PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-memory PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-memory PRIVATE)
else()
    target_link_libraries(idcu-memory PRIVATE pthread)
endif()

add_library(idcu::memory ALIAS idcu-memory)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
