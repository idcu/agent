cmake_minimum_required(VERSION 3.10)
project(demo-custom)

find_package(idcu-module-build REQUIRED)
idcu_build_module()

enable_testing()
add_executable(test_demo_custom tests/test_demo_custom.c)
target_link_libraries(test_demo_custom demo-custom)
add_test(NAME demo_custom_test COMMAND test_demo_custom)
