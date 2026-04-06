# Test No Interface Dynamic 业务模块 CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

set(MODULE_NAME idcu_business_dynamic_test_no_interface)

file(GLOB_RECURSE MODULE_SRCS src/*.c)

add_library(${MODULE_NAME} STATIC ${MODULE_SRCS})

target_include_directories(${MODULE_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../core/module-system/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../core/micro-kernel/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${MODULE_NAME} PRIVATE
    idcu::common
    idcu_core_sdk
    idcu_business_core_module
)
