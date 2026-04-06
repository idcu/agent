# 集成测试（单元测试已迁移到各个模块的 tests/ 目录）

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/modules
)

# 集成测试链接辅助函数
function(add_integration_test test_name source_file)
    add_executable(${test_name} ${source_file})
    
    if(WIN32)
        # MinGW 也支持 whole-archive，需要强制链接所有模块符号
        if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
            target_link_libraries(${test_name} PRIVATE -lws2_32
                -Wl,--whole-archive 
                idcu::common
                idcu_core_utils
                idcu_core_scheduler
                idcu_core_module_system
                idcu_core_micro_kernel
                idcu_services_monitor
                idcu_services_network
                idcu_services_security
                idcu_services_plugin
                idcu_core_test_framework
                idcu_business_core_module
                idcu_business_alert
                idcu_business_collect
                idcu_business_config
                idcu_business_data_collector
                idcu_business_healthcheck
                idcu_business_heartbeat
                idcu_business_log
                idcu_business_metrics
                -Wl,--no-whole-archive)
        else()
            target_link_libraries(${test_name} PRIVATE -lws2_32
                idcu::common
                idcu_core_utils
                idcu_core_scheduler
                idcu_core_module_system
                idcu_core_micro_kernel
                idcu_services_monitor
                idcu_services_network
                idcu_services_security
                idcu_services_plugin
                idcu_core_test_framework
                idcu_business_core_module
                idcu_business_alert
                idcu_business_collect
                idcu_business_config
                idcu_business_data_collector
                idcu_business_healthcheck
                idcu_business_heartbeat
                idcu_business_log
                idcu_business_metrics)
        endif()
    else()
        target_link_libraries(${test_name} PRIVATE pthread dl
            idcu_core_common
            idcu_core_utils
            idcu_core_scheduler
            idcu_core_module_system
            idcu_core_micro_kernel
            idcu_services_monitor
            idcu_services_network
            idcu_services_security
            idcu_services_plugin
            idcu_core_test_framework
            idcu_business_core_module
            idcu_business_alert
            idcu_business_collect
            idcu_business_config
            idcu_business_data_collector
            idcu_business_healthcheck
            idcu_business_heartbeat
            idcu_business_http_management
            idcu_business_log
            idcu_business_metrics)
    endif()
    
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# 集成测试
add_integration_test(test_module_comm integration/test_module_comm.c)
add_integration_test(test_error_recovery integration/test_error_recovery.c)
add_integration_test(test_module_load integration/test_module_load.c)
add_integration_test(test_micro_kernel_integration integration/test_micro_kernel_integration.c)
add_integration_test(test_distributed_integration integration/test_distributed_integration.c)

# 设置集成测试输出目录
set_target_properties(
    test_module_comm test_error_recovery test_module_load test_micro_kernel_integration test_distributed_integration
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out/tests
)
