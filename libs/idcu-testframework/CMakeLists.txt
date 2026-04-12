cmake_minimum_required(VERSION 3.15)
project(idcu-testframework C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-testframework STATIC
    src/idcu/testframework/testframework.c
)

target_include_directories(idcu-testframework PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_library(idcu::testframework ALIAS idcu-testframework)

if(BUILD_EXAMPLES)
    add_executable(example_simple_test examples/simple_test.c)
    target_link_libraries(example_simple_test PRIVATE idcu::testframework)
endif()
