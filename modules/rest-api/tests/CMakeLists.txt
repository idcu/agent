cmake_minimum_required(VERSION 3.15)

add_executable(test_rest_api test_rest_api.c)
target_link_libraries(test_rest_api idcu::rest-api idcu::testframework)
