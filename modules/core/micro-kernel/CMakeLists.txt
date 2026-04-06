# Micro Kernel 模块 CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 模块名称
set(MODULE_NAME idcu_core_micro_kernel)

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

# 依赖
target_link_libraries(${MODULE_NAME} PUBLIC
    idcu::common
    idcu::log
    idcu::config
    idcu::json
    idcu::network
    idcu::http-server
    idcu::http-client
    idcu::conn-pool
    idcu::distributed
    idcu::discovery
    idcu::metrics
    idcu::healthcheck
    idcu::alert
    idcu_core_scheduler
    idcu_core_module_system
    idcu_services_security
)

# 模块测试
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
