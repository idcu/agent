cmake_minimum_required(VERSION 3.15)
project(idcu-log C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-log STATIC
    src/idcu/log/log.c
)

target_include_directories(idcu-log PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-log PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-log PRIVATE)
else()
    target_link_libraries(idcu-log PRIVATE pthread)
endif()

add_library(idcu::log ALIAS idcu-log)

#if(BUILD_TESTS)
#    add_subdirectory(tests)
#endif()

