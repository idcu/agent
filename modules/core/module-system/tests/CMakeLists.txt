# Module System 模块测试 CMakeLists.txt

file(GLOB_RECURSE TEST_SRCS
    test_*.c
)

foreach(test_src ${TEST_SRCS})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_compile_definitions(${test_name} PRIVATE IDCU_NO_BUILD_TESTS)
    
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
            idcu_business_http_management
            idcu_business_log
            idcu_business_metrics)
    endif()
    
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
