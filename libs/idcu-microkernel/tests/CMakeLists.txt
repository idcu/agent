cmake_minimum_required(VERSION 3.15)

add_executable(test_microkernel test_microkernel.c)
target_link_libraries(test_microkernel idcu::microkernel idcu::testframework)
