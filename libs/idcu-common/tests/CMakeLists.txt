cmake_minimum_required(VERSION 3.10)
project(idcu-common-tests C)

set(CMAKE_C_STANDARD 99)

# 添加测试可执行文件
add_executable(test_atomic test_atomic.c)
target_link_libraries(test_atomic PRIVATE idcu::common)

add_executable(test_lock test_lock.c)
target_link_libraries(test_lock PRIVATE idcu::common)

add_executable(test_vector test_vector.c)
target_link_libraries(test_vector PRIVATE idcu::common)

add_executable(test_hash_map test_hash_map.c)
target_link_libraries(test_hash_map PRIVATE idcu::common)

add_executable(test_error_code test_error_code.c)
target_link_libraries(test_error_code PRIVATE idcu::common)

add_executable(test_linked_list test_linked_list.c)
target_link_libraries(test_linked_list PRIVATE idcu::common)

add_executable(test_string_buf test_string_buf.c)
target_link_libraries(test_string_buf PRIVATE idcu::common)

# 添加测试
enable_testing()
add_test(NAME test_atomic COMMAND test_atomic)
add_test(NAME test_lock COMMAND test_lock)
add_test(NAME test_vector COMMAND test_vector)
add_test(NAME test_hash_map COMMAND test_hash_map)
add_test(NAME test_error_code COMMAND test_error_code)
add_test(NAME test_linked_list COMMAND test_linked_list)
add_test(NAME test_string_buf COMMAND test_string_buf)
