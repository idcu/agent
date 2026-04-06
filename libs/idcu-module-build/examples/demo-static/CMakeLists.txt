# DEMO-STATIC: C STATIC LIBRARY PROJECT
cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../cmake")

include(idcu_module_build)

build_module()
