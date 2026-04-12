cmake_minimum_required(VERSION 3.15)
project(idcu-yaml C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-yaml STATIC
    src/idcu/yaml/yaml.c
)

target_include_directories(idcu-yaml PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-yaml PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-yaml PRIVATE)
else()
    target_link_libraries(idcu-yaml PRIVATE pthread)
endif()

add_library(idcu::yaml ALIAS idcu-yaml)

#if(BUILD_TESTS)
#    add_subdirectory(tests)
#endif()
