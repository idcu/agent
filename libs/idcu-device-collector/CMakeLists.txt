cmake_minimum_required(VERSION 3.15)
project(idcu-device-collector C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-device-collector STATIC
    src/idcu/device-collector/device-collector.c
)

target_include_directories(idcu-device-collector PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-device-collector PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-device-collector PRIVATE)
else()
    target_link_libraries(idcu-device-collector PRIVATE pthread)
endif()

add_library(idcu::device-collector ALIAS idcu-device-collector)
add_library(idcu::device_collector ALIAS idcu-device-collector)
