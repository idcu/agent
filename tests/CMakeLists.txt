set(TEST_COMMON_SRCS
    ../kernel/core/context.c
    ../kernel/infra/log.c
    ../kernel/concurrency/lock.c
    ../kernel/concurrency/atomic.c
    ../kernel/extension/test_framework.c
    ../kernel/infra/config_manager.c
    ../kernel/extension/dynamic_loader.c
    ../kernel/core/msg_bus.c
    ../kernel/core/coroutine.c
    ../kernel/core/module_registry.c
    ../kernel/extension/sandbox.c
    ../kernel/extension/sandbox_enhanced.c
    ../kernel/network/distributed_node.c
    ../kernel/extension/plugin_ecosystem.c
    ../kernel/concurrency/memory_pool.c
    ../kernel/infra/json_parser.c
    ../kernel/infra/health_check.c
    ../kernel/infra/metrics.c
    ../kernel/network/network_layer.c
    ../kernel/network/node_discovery.c
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

add_executable(test_dynamic_loader
    unit/test_dynamic_loader.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_dynamic_loader m pthread)
else()
    target_link_libraries(test_dynamic_loader kernel32 ws2_32)
endif()

add_test(NAME test_dynamic_loader COMMAND test_dynamic_loader)

add_executable(test_config_manager
    unit/test_config_manager.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_config_manager m pthread)
else()
    target_link_libraries(test_config_manager kernel32 ws2_32)
endif()

add_test(NAME test_config_manager COMMAND test_config_manager)

add_executable(test_msg_bus
    unit/test_msg_bus.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_msg_bus m pthread)
else()
    target_link_libraries(test_msg_bus kernel32 ws2_32)
endif()

add_test(NAME test_msg_bus COMMAND test_msg_bus)

add_executable(test_coroutine
    unit/test_coroutine.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_coroutine m pthread)
else()
    target_link_libraries(test_coroutine kernel32 ws2_32)
endif()

add_test(NAME test_coroutine COMMAND test_coroutine)

add_executable(test_registry
    unit/test_registry.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_registry m pthread)
else()
    target_link_libraries(test_registry kernel32 ws2_32)
endif()

add_test(NAME test_registry COMMAND test_registry)

add_executable(test_sandbox
    unit/test_sandbox.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_sandbox m pthread)
else()
    target_link_libraries(test_sandbox kernel32 ws2_32)
endif()

add_test(NAME test_sandbox COMMAND test_sandbox)

add_executable(test_sandbox_enhanced
    unit/test_sandbox_enhanced.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_sandbox_enhanced m pthread)
else()
    target_link_libraries(test_sandbox_enhanced kernel32 ws2_32)
endif()

add_test(NAME test_sandbox_enhanced COMMAND test_sandbox_enhanced)

add_executable(test_distributed_node
    unit/test_distributed_node.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_distributed_node m pthread)
else()
    target_link_libraries(test_distributed_node kernel32 ws2_32)
endif()

add_test(NAME test_distributed_node COMMAND test_distributed_node)

add_executable(test_plugin_ecosystem
    unit/test_plugin_ecosystem.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_plugin_ecosystem m pthread)
else()
    target_link_libraries(test_plugin_ecosystem kernel32 ws2_32)
endif()

add_test(NAME test_plugin_ecosystem COMMAND test_plugin_ecosystem)

add_executable(test_memory_pool
    unit/test_memory_pool.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_memory_pool m pthread)
else()
    target_link_libraries(test_memory_pool kernel32 ws2_32)
endif()

add_test(NAME test_memory_pool COMMAND test_memory_pool)

add_executable(test_json_parser
    unit/test_json_parser.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_json_parser m pthread)
else()
    target_link_libraries(test_json_parser kernel32 ws2_32)
endif()

add_test(NAME test_json_parser COMMAND test_json_parser)

add_executable(test_health_check
    unit/test_health_check.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_health_check m pthread)
else()
    target_link_libraries(test_health_check kernel32 ws2_32)
endif()

add_test(NAME test_health_check COMMAND test_health_check)

add_executable(test_metrics
    unit/test_metrics.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_metrics m pthread)
else()
    target_link_libraries(test_metrics kernel32 ws2_32)
endif()

add_test(NAME test_metrics COMMAND test_metrics)

add_executable(test_network_layer
    unit/test_network_layer.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_network_layer m pthread)
else()
    target_link_libraries(test_network_layer kernel32 ws2_32)
endif()

add_test(NAME test_network_layer COMMAND test_network_layer)

add_executable(test_node_discovery
    unit/test_node_discovery.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_node_discovery m pthread)
else()
    target_link_libraries(test_node_discovery kernel32 ws2_32)
endif()

add_test(NAME test_node_discovery COMMAND test_node_discovery)

set_target_properties(test_dynamic_loader test_config_manager
    test_msg_bus test_coroutine test_registry test_sandbox
    test_sandbox_enhanced test_distributed_node test_plugin_ecosystem
    test_memory_pool test_json_parser test_health_check test_metrics
    test_network_layer test_node_discovery
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out/tests
)

add_executable(test_module_comm
    integration/test_module_comm.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_module_comm m pthread)
else()
    target_link_libraries(test_module_comm kernel32 ws2_32)
endif()

add_test(NAME test_module_comm COMMAND test_module_comm)

add_executable(test_error_recovery
    integration/test_error_recovery.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_error_recovery m pthread)
else()
    target_link_libraries(test_error_recovery kernel32 ws2_32)
endif()

add_test(NAME test_error_recovery COMMAND test_error_recovery)

add_executable(test_module_load
    integration/test_module_load.c
    ${TEST_COMMON_SRCS}
)

if(NOT WIN32)
    target_link_libraries(test_module_load m pthread)
else()
    target_link_libraries(test_module_load kernel32 ws2_32)
endif()

add_test(NAME test_module_load COMMAND test_module_load)

set_target_properties(test_module_comm test_error_recovery test_module_load
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out/tests
)
