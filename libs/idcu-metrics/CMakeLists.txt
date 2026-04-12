cmake_minimum_required(VERSION 3.15)
project(idcu-metrics C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-metrics STATIC
    src/idcu/metrics/metrics.c
)

target_include_directories(idcu-metrics PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-metrics PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-metrics PRIVATE)
else()
    target_link_libraries(idcu-metrics PRIVATE pthread)
endif()

add_library(idcu::metrics ALIAS idcu-metrics)

#if(BUILD_TESTS)
#    add_subdirectory(tests)
#endif()
