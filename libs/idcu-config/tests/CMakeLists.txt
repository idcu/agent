cmake_minimum_required(VERSION 3.10)
project(idcu-config-tests C)

set(CMAKE_C_STANDARD 99)

# 添加测试可执行文件
add_executable(test_config_basic test_config_basic.c)
target_link_libraries(test_config_basic PRIVATE idcu::config idcu::common idcu::json)

add_executable(test_config_comprehensive test_config_comprehensive.c)
target_link_libraries(test_config_comprehensive PRIVATE idcu::config idcu::common idcu::json)

# 添加测试
enable_testing()
add_test(NAME test_config_basic COMMAND test_config_basic)
add_test(NAME test_config_comprehensive COMMAND test_config_comprehensive)
