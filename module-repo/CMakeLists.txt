
cmake_minimum_required(VERSION 3.10)
project(idcu_module_repo)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_SOURCE_DIR}/../include
)

set(CORE_MODULES
    official/core/core_module.c
    official/core/log_module.c
)

set(MONITORING_MODULES
    official/monitoring/health_check_module.c
    official/monitoring/metrics_module.c
    official/monitoring/prometheus_exporter_module.c
)

set(NETWORKING_MODULES
    official/networking/network_layer_module.c
    official/networking/node_discovery_module.c
    official/networking/distributed_node_module.c
)

set(SECURITY_MODULES
    official/security/sandbox_module.c
    official/security/sandbox_enhanced_module.c
)

set(ALL_MODULES
    ${CORE_MODULES}
    ${MONITORING_MODULES}
    ${NETWORKING_MODULES}
    ${SECURITY_MODULES}
)

add_library(idcu_modules STATIC ${ALL_MODULES})

option(BUILD_MODULES_DYNAMIC "Build modules as dynamic libraries" OFF)
if(BUILD_MODULES_DYNAMIC)
    foreach(module_src ${ALL_MODULES})
        get_filename_component(module_name ${module_src} NAME_WE)
        add_library(${module_name} SHARED ${module_src})
        set_target_properties(${module_name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/modules
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/modules
        )
    endforeach()
endif()
