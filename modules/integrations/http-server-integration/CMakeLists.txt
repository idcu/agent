# HTTP Server Integration Module CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

set(MODULE_NAME idcu_integrations_http_server)

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
    idcu::network
    idcu::http-server
    idcu::log
)
