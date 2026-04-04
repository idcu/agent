# Test Good 动态模块 CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

set(MODULE_NAME test_module_good)

file(GLOB_RECURSE MODULE_SRCS src/*.c)

add_library(${MODULE_NAME} SHARED ${MODULE_SRCS})

set_target_properties(${MODULE_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out
    PREFIX ""
)

target_include_directories(${MODULE_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(UNIX AND NOT APPLE)
    target_compile_options(${MODULE_NAME} PRIVATE -fvisibility=hidden)
endif()

target_link_libraries(${MODULE_NAME} PRIVATE
    idcu_core_common
    idcu_core_utils
    idcu_core_module_system
)
