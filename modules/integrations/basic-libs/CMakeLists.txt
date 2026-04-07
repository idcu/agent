cmake_minimum_required(VERSION 3.10)
project(idcu_basic_libs C)

set(CMAKE_C_STANDARD 11)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/libs/idcu-common/include
    ${CMAKE_SOURCE_DIR}/libs/idcu-log/include
    ${CMAKE_SOURCE_DIR}/libs/idcu-config/include
    ${CMAKE_SOURCE_DIR}/libs/idcu-json/include
    ${CMAKE_SOURCE_DIR}/libs/idcu-metrics/include
    ${CMAKE_SOURCE_DIR}/libs/idcu-healthcheck/include
    ${CMAKE_SOURCE_DIR}/libs/idcu-http-server/include
    ${CMAKE_SOURCE_DIR}/libs/idcu-http-client/include
    ${CMAKE_SOURCE_DIR}/libs/idcu-network/include
    ${CMAKE_SOURCE_DIR}/modules/core/module-system/include
)

set(SOURCES
    src/basic_libs.c
)

add_library(idcu_basic_libs STATIC ${SOURCES})

target_link_libraries(idcu_basic_libs
    idcu_common
    idcu_log
    idcu_config
    idcu_json
    idcu_metrics
    idcu_healthcheck
    idcu_http_server
    idcu_http_client
    idcu_network
    idcu_module_system
)
