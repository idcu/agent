cmake_minimum_required(VERSION 3.10)

# 包含 idcu-module-build 系统
include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/idcu_module_build.cmake")

# 构建模块
build_module(
    MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    NAMESPACE "demo"
)
