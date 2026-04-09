cmake_minimum_required(VERSION 3.15)

add_executable(test_msgbus test_msgbus.c)
target_link_libraries(test_msgbus idcu::msgbus idcu::testframework)
