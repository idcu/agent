cmake_minimum_required(VERSION 3.15)
project(idcu-module-isolation C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-module-isolation STATIC
    src/idcu/module-isolation/module-isolation.c
)

target_include_directories(idcu-module-isolation PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-module-isolation PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-module-isolation PRIVATE)
else()
    target_link_libraries(idcu-module-isolation PRIVATE pthread)
endif()

add_library(idcu::module-isolation ALIAS idcu-module-isolation)
