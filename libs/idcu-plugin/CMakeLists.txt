cmake_minimum_required(VERSION 3.15)
project(idcu-plugin VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-plugin STATIC
    src/idcu/plugin/plugin.c
)

target_include_directories(idcu-plugin PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-plugin PRIVATE idcu::common)
add_library(idcu::plugin ALIAS idcu-plugin)

if(WIN32)
    target_link_libraries(idcu-plugin PRIVATE psapi)
endif()

if(BUILD_TESTING AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt)
    add_subdirectory(examples)
endif()
