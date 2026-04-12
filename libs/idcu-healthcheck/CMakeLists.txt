cmake_minimum_required(VERSION 3.15)
project(idcu-healthcheck C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-healthcheck STATIC
    src/idcu/healthcheck/healthcheck.c
)

target_include_directories(idcu-healthcheck PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-healthcheck PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-healthcheck PRIVATE)
else()
    target_link_libraries(idcu-healthcheck PRIVATE pthread)
endif()

add_library(idcu::healthcheck ALIAS idcu-healthcheck)

#if(BUILD_TESTS)
#    add_subdirectory(tests)
#endif()
