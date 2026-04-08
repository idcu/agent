# 超简单的 CMakeLists.txt - 小白专用！
# 只需要这两行，其他什么都不用管！

cmake_minimum_required(VERSION 3.10)

# 包含极简构建系统（根据你的实际路径修改）
include("../../cmake/idcu_module_build_minimal.cmake")

# 开始构建！就这么简单！
build()
