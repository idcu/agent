cmake_minimum_required(VERSION 3.10)
project(idcu_agent C)
set(CMAKE_C_STANDARD 99)

set(IDCU_VERSION_MAJOR 1)
set(IDCU_VERSION_MINOR 0)
set(IDCU_VERSION_PATCH 0)
set(IDCU_VERSION "${IDCU_VERSION_MAJOR}.${IDCU_VERSION_MINOR}.${IDCU_VERSION_PATCH}")

if(MSVC)
    add_compile_options(/W4)
elseif(MINGW)
    add_compile_options(-O2 -Wall -Wextra)
else()
    add_compile_options(-O2 -Wall -Wextra)
endif()



# ==================== 独立库 ====================
add_subdirectory(libs/idcu-module-build)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/idcu-module-build/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/libs/idcu-module-build" ${CMAKE_PREFIX_PATH})

# 暂时禁用示例构建，专注于核心库
set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)

add_subdirectory(libs/idcu-common)
add_subdirectory(libs/idcu-log)
add_subdirectory(libs/idcu-network)
add_subdirectory(libs/idcu-http-server)
add_subdirectory(libs/idcu-http-client)
add_subdirectory(libs/idcu-json)
add_subdirectory(libs/idcu-config)
add_subdirectory(libs/idcu-conn-pool)
add_subdirectory(libs/idcu-distributed)
add_subdirectory(libs/idcu-discovery)
add_subdirectory(libs/idcu-metrics)
add_subdirectory(libs/idcu-healthcheck)
add_subdirectory(libs/idcu-alert)
add_subdirectory(libs/idcu-memory)
add_subdirectory(libs/idcu-permission)
add_subdirectory(libs/idcu-sandbox)
add_subdirectory(libs/idcu-coroutine)
add_subdirectory(libs/idcu-msgbus)
add_subdirectory(libs/idcu-storage)
add_subdirectory(libs/idcu-cache)
add_subdirectory(libs/idcu-module-system)
add_subdirectory(libs/idcu-testframework)
add_subdirectory(libs/idcu-plugin)
add_subdirectory(libs/idcu-security-enhanced)

# ==================== 核心模块 ====================
add_subdirectory(modules/core/test-framework)
add_subdirectory(modules/core/sdk)
add_subdirectory(modules/core/scheduler)
add_subdirectory(modules/core/module-system)
add_subdirectory(modules/core/micro-kernel)

# ==================== 服务模块 ====================
add_subdirectory(modules/services/storage)
add_subdirectory(modules/services/cache)
add_subdirectory(modules/services/security)
add_subdirectory(modules/services/plugin)

# ==================== 集成模块 ====================
add_subdirectory(modules/integrations/network-monitor-integration)
add_subdirectory(modules/integrations/log-integration)
add_subdirectory(modules/integrations/config-integration)
add_subdirectory(modules/integrations/json-integration)
add_subdirectory(modules/integrations/network-integration)
add_subdirectory(modules/integrations/http-server-integration)
add_subdirectory(modules/integrations/http-client-integration)
add_subdirectory(modules/integrations/metrics-integration)
add_subdirectory(modules/integrations/healthcheck-integration)

# ==================== 业务模块 ====================
add_subdirectory(modules/business/core-module)
add_subdirectory(modules/business/alert)
add_subdirectory(modules/business/collect)
add_subdirectory(modules/business/config)
add_subdirectory(modules/business/data-collector)
add_subdirectory(modules/business/healthcheck)
add_subdirectory(modules/business/heartbeat)
# add_subdirectory(modules/business/http-management)  # 暂时禁用，需要完整的 management_api 实现
add_subdirectory(modules/business/http-client)
add_subdirectory(modules/business/log)
add_subdirectory(modules/business/metrics)
add_subdirectory(modules/business/storage)
add_subdirectory(modules/business/cache)
add_subdirectory(modules/business/security)
add_subdirectory(modules/business/examples/simple-example)
add_subdirectory(modules/business/examples/messaging-example)
# add_subdirectory(modules/business/examples/advanced-example)  # 暂时禁用，需要完整的依赖

# ==================== 动态模块 ====================
option(BUILD_SAMPLE_DLL "Build sample dynamic module" ON)
if(BUILD_SAMPLE_DLL)
    add_subdirectory(modules/business/dynamic/sample)
endif()

if(BUILD_TESTS)
    add_subdirectory(modules/business/dynamic/test-good)
    add_subdirectory(modules/business/dynamic/test-init-fail)
    add_subdirectory(modules/business/dynamic/test-no-interface)
endif()

# ==================== 主程序 ====================
add_executable(idcu_agent app/main.c)
set_target_properties(idcu_agent PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out
)

if(WIN32)
    # MinGW 也支持 whole-archive，需要强制链接所有模块符号
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_link_libraries(idcu_agent PRIVATE -lws2_32
            -Wl,--whole-archive 
            idcu::common
            idcu::log
            idcu::json
            idcu::config
            idcu::network
            idcu::http-server
            idcu::http-client
            idcu::conn-pool
            idcu::distributed
            idcu::discovery
            idcu::metrics
            idcu::healthcheck
            idcu::alert
            idcu_core_scheduler
            idcu_core_module_system
            idcu_core_micro_kernel
            idcu_services_storage
            idcu_services_cache
            idcu_services_security
            idcu_services_plugin
            idcu_integrations_network_monitor
            idcu_integrations_log
            idcu_integrations_config
            idcu_integrations_json
            idcu_integrations_network
            idcu_integrations_http_server
            idcu_integrations_http_client
            idcu_integrations_metrics
            idcu_integrations_healthcheck
            idcu_business_core_module
            idcu_business_alert
            idcu_business_collect
            idcu_business_config
            idcu_business_data_collector
            idcu_business_healthcheck
            idcu_business_heartbeat
            idcu_business_http_client
            idcu_business_log
            idcu_business_metrics
            idcu_business_storage
            idcu_business_cache
            idcu_business_security
            -Wl,--no-whole-archive)
    else()
        target_link_libraries(idcu_agent PRIVATE -lws2_32
            idcu::common
            idcu::log
            idcu::json
            idcu::config
            idcu::network
            idcu::http-server
            idcu::http-client
            idcu::conn-pool
            idcu::distributed
            idcu::discovery
            idcu::metrics
            idcu::healthcheck
            idcu::alert
            idcu_core_scheduler
            idcu_core_module_system
            idcu_core_micro_kernel
            idcu_services_storage
            idcu_services_cache
            idcu_services_security
            idcu_services_plugin
            idcu_integrations_network_monitor
            idcu_integrations_log
            idcu_integrations_config
            idcu_integrations_json
            idcu_integrations_network
            idcu_integrations_http_server
            idcu_integrations_http_client
            idcu_integrations_metrics
            idcu_integrations_healthcheck
            idcu_business_core_module
            idcu_business_alert
            idcu_business_collect
            idcu_business_config
            idcu_business_data_collector
            idcu_business_healthcheck
            idcu_business_heartbeat
            idcu_business_http_client
            idcu_business_log
            idcu_business_metrics
            idcu_business_storage
            idcu_business_cache)
    endif()
else()
    target_link_libraries(idcu_agent PRIVATE pthread dl)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_link_options(idcu_agent PRIVATE -Wl,--whole-archive 
            idcu::common
            idcu::log
            idcu::json
            idcu::config
            idcu::network
            idcu::http-server
            idcu::http-client
            idcu::conn-pool
            idcu::distributed
            idcu::discovery
            idcu::metrics
            idcu::healthcheck
            idcu::alert
            idcu_core_scheduler
            idcu_core_module_system
            idcu_core_micro_kernel
            idcu_services_storage
            idcu_services_cache
            idcu_services_security
            idcu_services_plugin
            idcu_integrations_network_monitor
            idcu_integrations_log
            idcu_integrations_config
            idcu_integrations_json
            idcu_integrations_network
            idcu_integrations_http_server
            idcu_integrations_http_client
            idcu_integrations_metrics
            idcu_integrations_healthcheck
            idcu_business_core_module
            idcu_business_alert
            idcu_business_collect
            idcu_business_config
            idcu_business_data_collector
            idcu_business_healthcheck
            idcu_business_heartbeat
            idcu_business_http_client
            idcu_business_log
            idcu_business_metrics
            idcu_business_storage
            idcu_business_cache
            idcu_business_security
            -Wl,--no-whole-archive)
    else()
        target_link_libraries(idcu_agent PRIVATE
            idcu::common
            idcu::log
            idcu::json
            idcu::config
            idcu::network
            idcu::http-server
            idcu::http-client
            idcu::conn-pool
            idcu::distributed
            idcu::discovery
            idcu::metrics
            idcu::healthcheck
            idcu::alert
            idcu_core_scheduler
            idcu_core_module_system
            idcu_core_micro_kernel
            idcu_services_storage
            idcu_services_cache
            idcu_services_security
            idcu_services_plugin
            idcu_integrations_network_monitor
            idcu_integrations_log
            idcu_integrations_config
            idcu_integrations_json
            idcu_integrations_network
            idcu_integrations_http_server
            idcu_integrations_http_client
            idcu_integrations_metrics
            idcu_integrations_healthcheck
            idcu_business_core_module
            idcu_business_alert
            idcu_business_collect
            idcu_business_config
            idcu_business_data_collector
            idcu_business_healthcheck
            idcu_business_heartbeat
            idcu_business_http_client
            idcu_business_log
            idcu_business_metrics
            idcu_business_storage
            idcu_business_cache)
    endif()
endif()

# ==================== 测试和基准测试 ====================
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_BENCHMARKS "Build benchmarks" ON)
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)
option(ENABLE_CODE_COVERAGE "Enable code coverage with gcov/lcov" OFF)

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
        set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy not found, static analysis disabled")
    endif()
endif()

if(ENABLE_CODE_COVERAGE)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "Code coverage requires GCC or Clang compiler")
    endif()
    if(NOT UNIX)
        message(FATAL_ERROR "Code coverage is only supported on Unix-like systems (Linux/macOS)")
    endif()
    message(STATUS "Code coverage enabled")
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
    find_program(LCOV_EXE NAMES lcov)
    find_program(GENHTML_EXE NAMES genhtml)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    
    if(ENABLE_CODE_COVERAGE)
        add_custom_target(coverage_clean
            COMMAND ${LCOV_EXE} --directory ${PROJECT_BINARY_DIR} --zerocounters
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
            COMMENT "Clearing coverage counters"
        )
        add_custom_target(coverage_capture
            COMMAND ${LCOV_EXE} --directory ${PROJECT_BINARY_DIR} --capture --output-file coverage.info
            COMMAND ${LCOV_EXE} --remove coverage.info '/usr/*' '*/tests/*' '*/modules/*' --output-file coverage_filtered.info
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
            COMMENT "Capturing coverage data"
            DEPENDS coverage_clean
        )
        if(GENHTML_EXE)
            add_custom_target(coverage_report
                COMMAND ${GENHTML_EXE} -o coverage_report coverage_filtered.info
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                COMMENT "Generating HTML coverage report"
                DEPENDS coverage_capture
            )
            add_custom_target(coverage
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                COMMAND ${CMAKE_COMMAND} --build . --target coverage_report
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                COMMENT "Running tests and generating coverage report"
                DEPENDS coverage_clean
            )
        else()
            add_custom_target(coverage
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                COMMAND ${CMAKE_COMMAND} --build . --target coverage_capture
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                COMMENT "Running tests and capturing coverage data"
                DEPENDS coverage_clean
            )
        endif()
    endif()
endif()

if(BUILD_BENCHMARKS)
    add_executable(idcu_benchmark app/benchmark.c)
    set_target_properties(idcu_benchmark PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out
    )
    target_include_directories(idcu_benchmark PRIVATE
        ${PROJECT_SOURCE_DIR}/modules/core/scheduler/include
        ${PROJECT_SOURCE_DIR}/libs/idcu-memory/include
    )
    target_link_libraries(idcu_benchmark PRIVATE
        idcu::common
        idcu::memory
        idcu_core_micro_kernel
        idcu_core_scheduler
        idcu_business_core_module
    )
    if(NOT WIN32)
        target_link_libraries(idcu_benchmark PRIVATE pthread m)
    else()
        if(MINGW)
            target_link_libraries(idcu_benchmark PRIVATE m)
        endif()
    endif()
endif()

# ==================== 动态模块示例 ====================
# 动态模块通过子目录的 CMakeLists.txt 管理

# ==================== 示例程序 ====================
# 暂时禁用，需要更新 examples 目录结构
option(BUILD_EXAMPLES "Build example programs" OFF)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
