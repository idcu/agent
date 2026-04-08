cmake_minimum_required(VERSION 3.10)
project(idcu-log-tests C)

set(CMAKE_C_STANDARD 99)

# 添加测试可执行文件
add_executable(test_log_init test_log_init.c)
target_link_libraries(test_log_init PRIVATE idcu::log idcu::common)

add_executable(test_log_level test_log_level.c)
target_link_libraries(test_log_level PRIVATE idcu::log idcu::common)

add_executable(test_log_format test_log_format.c)
target_link_libraries(test_log_format PRIVATE idcu::log idcu::common)

add_executable(test_log_file test_log_file.c)
target_link_libraries(test_log_file PRIVATE idcu::log idcu::common)

add_executable(test_log_comprehensive test_log_comprehensive.c)
target_link_libraries(test_log_comprehensive PRIVATE idcu::log idcu::common)

# 添加测试
enable_testing()
add_test(NAME test_log_init COMMAND test_log_init)
add_test(NAME test_log_level COMMAND test_log_level)
add_test(NAME test_log_format COMMAND test_log_format)
add_test(NAME test_log_file COMMAND test_log_file)
add_test(NAME test_log_comprehensive COMMAND test_log_comprehensive)
