cmake_minimum_required(VERSION 3.15)

add_executable(test_config test_config.c)
target_link_libraries(test_config PRIVATE idcu::config idcu::testframework)

add_test(NAME test_config COMMAND test_config)
