# Cache 服务模块 CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

set(MODULE_NAME idcu_services_cache)

file(GLOB_RECURSE MODULE_SRCS src/*.c)

add_library(${MODULE_NAME} STATIC ${MODULE_SRCS})

target_include_directories(${MODULE_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../core/common/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../core/utils/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${MODULE_NAME} PUBLIC
    idcu_core_common
    idcu_core_utils
)
