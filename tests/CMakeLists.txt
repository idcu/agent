include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/modules
)

# 单元测试
add_executable(test_config_manager
    unit/test_config_manager.c
)
target_link_libraries(test_config_manager idcu_core)
if(NOT WIN32)
    target_link_libraries(test_config_manager pthread)
endif()
add_test(NAME test_config_manager COMMAND test_config_manager)

add_executable(test_msg_bus
    unit/test_msg_bus.c
)
target_link_libraries(test_msg_bus idcu_core)
if(NOT WIN32)
    target_link_libraries(test_msg_bus pthread)
endif()
add_test(NAME test_msg_bus COMMAND test_msg_bus)

add_executable(test_coroutine
    unit/test_coroutine.c
)
target_link_libraries(test_coroutine idcu_core)
if(NOT WIN32)
    target_link_libraries(test_coroutine pthread)
endif()
add_test(NAME test_coroutine COMMAND test_coroutine)

add_executable(test_registry
    unit/test_registry.c
)
target_link_libraries(test_registry idcu_core)
if(NOT WIN32)
    target_link_libraries(test_registry pthread)
endif()
add_test(NAME test_registry COMMAND test_registry)

add_executable(test_sandbox
    unit/test_sandbox.c
)
target_link_libraries(test_sandbox idcu_core)
if(NOT WIN32)
    target_link_libraries(test_sandbox pthread)
endif()
add_test(NAME test_sandbox COMMAND test_sandbox)

add_executable(test_sandbox_enhanced
    unit/test_sandbox_enhanced.c
)
target_link_libraries(test_sandbox_enhanced idcu_core)
if(NOT WIN32)
    target_link_libraries(test_sandbox_enhanced pthread)
endif()
add_test(NAME test_sandbox_enhanced COMMAND test_sandbox_enhanced)

add_executable(test_distributed_node
    unit/test_distributed_node.c
)
target_link_libraries(test_distributed_node idcu_core)
if(NOT WIN32)
    target_link_libraries(test_distributed_node pthread)
endif()
add_test(NAME test_distributed_node COMMAND test_distributed_node)

add_executable(test_plugin_ecosystem
    unit/test_plugin_ecosystem.c
)
target_link_libraries(test_plugin_ecosystem idcu_core)
if(NOT WIN32)
    target_link_libraries(test_plugin_ecosystem pthread)
endif()
add_test(NAME test_plugin_ecosystem COMMAND test_plugin_ecosystem)

add_executable(test_memory_pool
    unit/test_memory_pool.c
)
target_link_libraries(test_memory_pool idcu_core)
if(NOT WIN32)
    target_link_libraries(test_memory_pool pthread)
endif()
add_test(NAME test_memory_pool COMMAND test_memory_pool)

add_executable(test_json_parser
    unit/test_json_parser.c
)
target_link_libraries(test_json_parser idcu_core)
if(NOT WIN32)
    target_link_libraries(test_json_parser pthread)
endif()
add_test(NAME test_json_parser COMMAND test_json_parser)

add_executable(test_health_check
    unit/test_health_check.c
)
target_link_libraries(test_health_check idcu_core)
if(NOT WIN32)
    target_link_libraries(test_health_check pthread)
endif()
add_test(NAME test_health_check COMMAND test_health_check)

add_executable(test_metrics
    unit/test_metrics.c
)
target_link_libraries(test_metrics idcu_core)
if(NOT WIN32)
    target_link_libraries(test_metrics pthread)
endif()
add_test(NAME test_metrics COMMAND test_metrics)

add_executable(test_network_layer
    unit/test_network_layer.c
)
target_link_libraries(test_network_layer idcu_core)
if(NOT WIN32)
    target_link_libraries(test_network_layer pthread)
else()
    target_link_libraries(test_network_layer ws2_32)
endif()
add_test(NAME test_network_layer COMMAND test_network_layer)

add_executable(test_node_discovery
    unit/test_node_discovery.c
)
target_link_libraries(test_node_discovery idcu_core)
if(NOT WIN32)
    target_link_libraries(test_node_discovery pthread)
else()
    target_link_libraries(test_node_discovery ws2_32)
endif()
add_test(NAME test_node_discovery COMMAND test_node_discovery)

add_executable(test_dynamic_loader
    unit/test_dynamic_loader.c
)
target_link_libraries(test_dynamic_loader idcu_core)
if(NOT WIN32)
    target_link_libraries(test_dynamic_loader pthread dl)
else()
    target_link_libraries(test_dynamic_loader)
endif()
add_test(NAME test_dynamic_loader COMMAND test_dynamic_loader)

# 设置单元测试输出目录
set_target_properties(
    test_config_manager test_msg_bus test_coroutine test_registry
    test_sandbox test_sandbox_enhanced test_distributed_node
    test_plugin_ecosystem test_memory_pool test_json_parser
    test_health_check test_metrics test_network_layer
    test_node_discovery test_dynamic_loader
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out/tests
)

# 集成测试
add_executable(test_module_comm
    integration/test_module_comm.c
)
target_link_libraries(test_module_comm idcu_core)
if(NOT WIN32)
    target_link_libraries(test_module_comm pthread)
endif()
add_test(NAME test_module_comm COMMAND test_module_comm)

add_executable(test_error_recovery
    integration/test_error_recovery.c
)
target_link_libraries(test_error_recovery idcu_core)
if(NOT WIN32)
    target_link_libraries(test_error_recovery pthread)
endif()
add_test(NAME test_error_recovery COMMAND test_error_recovery)

add_executable(test_module_load
    integration/test_module_load.c
)
target_link_libraries(test_module_load idcu_core)
if(NOT WIN32)
    target_link_libraries(test_module_load pthread)
endif()
add_test(NAME test_module_load COMMAND test_module_load)

# 设置集成测试输出目录
set_target_properties(
    test_module_comm test_error_recovery test_module_load
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out/tests
)
