cmake_minimum_required(VERSION 3.10)

# Include the idcu-module-build system
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
include(idcu_module_build)

# Enable plugins
enable_plugin(version_generator)
enable_plugin(signer)
enable_plugin(coverage)
enable_plugin(packager)
enable_plugin(publisher)

# Build the module
build_module(MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Add include directory for generated version header
target_include_directories(demo-plugins PRIVATE "${CMAKE_BINARY_DIR}/include")
