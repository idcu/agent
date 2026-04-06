# Network-Monitor Integration Module CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

set(MODULE_NAME idcu_integrations_network_monitor)

file(GLOB_RECURSE MODULE_SRCS src/*.c)

add_library(${MODULE_NAME} STATIC ${MODULE_SRCS})

target_include_directories(${MODULE_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../core/module-system/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../core/micro-kernel/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${MODULE_NAME} PUBLIC
    idcu::common
    idcu::log
    idcu::network
    idcu::http-server
    idcu::http-client
    idcu::conn-pool
    idcu::distributed
    idcu::discovery
    idcu::metrics
    idcu::healthcheck
    idcu::alert
)
