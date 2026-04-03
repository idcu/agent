set(EXAMPLE_COMMON_SRCS
    ../kernel/core/context.c
    ../kernel/infra/log.c
    ../kernel/concurrency/lock.c
    ../kernel/concurrency/atomic.c
    ../kernel/infra/config_manager.c
    ../kernel/network/distributed_node.c
    ../kernel/network/node_discovery.c
    ../kernel/extension/plugin_ecosystem.c
    ../kernel/infra/health_check.c
    ../kernel/infra/metrics.c
    ../kernel/network/network_layer.c
)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/common
    ${PROJECT_SOURCE_DIR}/include/concurrency
    ${PROJECT_SOURCE_DIR}/include/extension
    ${PROJECT_SOURCE_DIR}/include/infra
    ${PROJECT_SOURCE_DIR}/include/network
    ${PROJECT_SOURCE_DIR}/kernel
)

add_executable(distributed_app
    distributed_app.c
    ${EXAMPLE_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(distributed_app m pthread)
else()
    target_link_libraries(distributed_app kernel32 ws2_32)
endif()

set_target_properties(distributed_app
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out/examples
)
