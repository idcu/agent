cmake_minimum_required(VERSION 3.15)
project(idcu-storage-module C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-storage-module STATIC
    src/idcu/storage_module/storage_module.c
)

target_include_directories(idcu-storage-module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-storage-module PUBLIC
    idcu::common
    idcu::storage
    idcu::core_module
)

if(WIN32)
    target_link_libraries(idcu-storage-module PRIVATE)
else()
    target_link_libraries(idcu-storage-module PRIVATE pthread)
endif()

add_library(idcu::storage_module ALIAS idcu-storage-module)
