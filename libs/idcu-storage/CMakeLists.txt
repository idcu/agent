cmake_minimum_required(VERSION 3.15)
project(idcu-storage C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-storage STATIC
    src/idcu/storage/storage.c
)

target_include_directories(idcu-storage PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-storage PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-storage PRIVATE)
else()
    target_link_libraries(idcu-storage PRIVATE pthread)
endif()

add_library(idcu::storage ALIAS idcu-storage)
