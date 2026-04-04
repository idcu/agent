# SDK 模块 CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 模块名称
set(MODULE_NAME idcu_core_sdk)

# 收集源文件
file(GLOB_RECURSE MODULE_SRCS
    src/*.c
)

# 创建静态库
add_library(${MODULE_NAME} STATIC ${MODULE_SRCS})

# 公共头文件目录
target_include_directories(${MODULE_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 私有依赖
target_link_libraries(${MODULE_NAME} PRIVATE
    idcu_core_common
    idcu_core_utils
    idcu_core_module_system
    idcu_core_scheduler
)


