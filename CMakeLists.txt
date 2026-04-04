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



# ==================== 核心模块 ====================
add_subdirectory(modules/core/common)
add_subdirectory(modules/core/utils)
add_subdirectory(modules/core/test-framework)
add_subdirectory(modules/core/sdk)
add_subdirectory(modules/core/scheduler)
add_subdirectory(modules/core/module-system)
add_subdirectory(modules/core/micro-kernel)

# ==================== 服务模块 ====================
add_subdirectory(modules/services/monitor)
add_subdirectory(modules/services/network)
add_subdirectory(modules/services/security)
add_subdirectory(modules/services/plugin)

# ==================== 业务模块 ====================
add_subdirectory(modules/business/core-module)
add_subdirectory(modules/business/alert)
add_subdirectory(modules/business/collect)
add_subdirectory(modules/business/config)
add_subdirectory(modules/business/data-collector)
add_subdirectory(modules/business/healthcheck)
add_subdirectory(modules/business/heartbeat)
# add_subdirectory(modules/business/http-management)
add_subdirectory(modules/business/log)
add_subdirectory(modules/business/metrics)
add_subdirectory(modules/business/examples/simple-example)
add_subdirectory(modules/business/examples/messaging-example)
add_subdirectory(modules/business/examples/advanced-example)

# ==================== 动态模块 ====================
# 暂时禁用，需要修复 TMP/TEMP 环境变量问题
option(BUILD_SAMPLE_DLL "Build sample dynamic module" OFF)
if(BUILD_SAMPLE_DLL)
    add_subdirectory(modules/business/dynamic/sample)
endif()

if(BUILD_TESTS)
    # 暂时禁用，需要修复 TMP/TEMP 环境变量问题
    # add_subdirectory(modules/business/dynamic/test-good)
    # add_subdirectory(modules/business/dynamic/test-init-fail)
    # add_subdirectory(modules/business/dynamic/test-no-interface)
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
            idcu_core_common
            idcu_core_utils
            idcu_core_scheduler
            idcu_core_module_system
            idcu_core_micro_kernel
            idcu_services_monitor
            idcu_services_network
            idcu_services_security
            idcu_services_plugin
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
        target_link_libraries(idcu_agent PRIVATE -lws2_32
            idcu_core_common
            idcu_core_utils
            idcu_core_scheduler
            idcu_core_module_system
            idcu_core_micro_kernel
            idcu_services_monitor
            idcu_services_network
            idcu_services_security
            idcu_services_plugin
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
    target_link_libraries(idcu_agent PRIVATE pthread dl)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_link_options(idcu_agent PRIVATE -Wl,--whole-archive 
            idcu_core_common
            idcu_core_utils
            idcu_core_scheduler
            idcu_core_module_system
            idcu_core_micro_kernel
            idcu_services_monitor
            idcu_services_network
            idcu_services_security
            idcu_services_plugin
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
        target_link_libraries(idcu_agent PRIVATE
            idcu_core_common
            idcu_core_utils
            idcu_core_scheduler
            idcu_core_module_system
            idcu_core_micro_kernel
            idcu_services_monitor
            idcu_services_network
            idcu_services_security
            idcu_services_plugin
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
    target_link_libraries(idcu_benchmark PRIVATE
        idcu_core_common
        idcu_core_utils
        idcu_core_micro_kernel
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

option(BUILD_DYNAMIC_LOADER_EXAMPLE "Build dynamic loader example" ON)
if(BUILD_DYNAMIC_LOADER_EXAMPLE)
    add_executable(dynamic_module_example examples/dynamic_module_example.c)
    set_target_properties(dynamic_module_example PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out
    )
    target_link_libraries(dynamic_module_example PRIVATE
        idcu_core_common
        idcu_core_utils
        idcu_core_module_system
    )
    if(NOT WIN32)
        target_link_libraries(dynamic_module_example PRIVATE pthread dl)
    endif()
endif()

option(BUILD_PROMETHEUS_EXPORTER_EXAMPLE "Build Prometheus exporter example" ON)
if(BUILD_PROMETHEUS_EXPORTER_EXAMPLE)
    add_executable(prometheus_exporter_example examples/prometheus_exporter_example.c)
    set_target_properties(prometheus_exporter_example PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out
    )
    target_link_libraries(prometheus_exporter_example PRIVATE
        idcu_core_common
        idcu_core_utils
        idcu_services_monitor
    )
    if(NOT WIN32)
        target_link_libraries(prometheus_exporter_example PRIVATE pthread)
    else()
        target_link_libraries(prometheus_exporter_example PRIVATE -lws2_32)
    endif()
endif()
