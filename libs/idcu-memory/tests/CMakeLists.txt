cmake_minimum_required(VERSION 3.15)
project(idcu-memory-tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(test_memory
    test_memory.c
)

target_link_libraries(test_memory PRIVATE
    idcu::memory
    idcu::testframework
)

enable_testing()
add_test(NAME idcu-memory-tests COMMAND test_memory)
