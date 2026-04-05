# Network 服务模块 CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

set(MODULE_NAME idcu_services_network)

# 收集源文件，暂时排除 management_api.c 以避免循环依赖
file(GLOB MODULE_SRCS
    src/connection_pool.c
    src/distributed_node.c
    src/http_client.c
    src/http_server.c
    src/network_layer.c
    src/node_discovery.c
)

add_library(${MODULE_NAME} STATIC ${MODULE_SRCS})

target_include_directories(${MODULE_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../monitor/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${MODULE_NAME} PUBLIC
    idcu_core_common
    idcu_core_utils
    idcu_core_module_system
    idcu_core_micro_kernel
    idcu_services_monitor
)

if(WIN32)
    target_link_libraries(${MODULE_NAME} PRIVATE -lws2_32)
endif()

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
