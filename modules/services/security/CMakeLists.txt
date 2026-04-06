cmake_minimum_required(VERSION 3.10)
include("${CMAKE_SOURCE_DIR}/libs/idcu-module-build/cmake/idcu_module_build.cmake")
idcu_build_module()

# 模块测试 (暂时禁用，等待测试框架问题修复)
#if(BUILD_TESTS)
#    add_subdirectory(tests)
#endif()
