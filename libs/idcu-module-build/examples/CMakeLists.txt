cmake_minimum_required(VERSION 3.10)

project(idcu-module-build-examples VERSION 1.0.0 LANGUAGES C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")

include(idcu_module_build)

message(STATUS "Building IDCU Module Build Examples")

add_subdirectory(demo-shared)
add_subdirectory(demo-static)
