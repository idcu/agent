cmake_minimum_required(VERSION 3.15)

add_executable(test_sdk test_sdk.c)
target_link_libraries(test_sdk idcu::sdk idcu::testframework)
