add_library(idcu_module_build INTERFACE)

target_include_directories(idcu_module_build INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_library(idcu::module_build ALIAS idcu_module_build)

message(STATUS "idcu-module-build initialized")
