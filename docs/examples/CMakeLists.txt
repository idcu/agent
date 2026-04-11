cmake_minimum_required(VERSION 3.10)
project(idcu_examples)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find idcu packages
find_package(idcu-common REQUIRED)
find_package(idcu-log REQUIRED)
find_package(idcu-json REQUIRED)
find_package(idcu-yaml REQUIRED)
find_package(idcu-network REQUIRED)
find_package(idcu-module-system REQUIRED)
find_package(idcu-microkernel REQUIRED)
find_package(idcu-config REQUIRED)
find_package(idcu-metrics REQUIRED)
find_package(idcu-http-server REQUIRED)

# Example 1: Logging
add_executable(example_logging example_logging.c)
target_link_libraries(example_logging
    idcu-common::idcu-common
    idcu-log::idcu-log
)

# Example 2: Config
add_executable(example_config example_config.c)
target_link_libraries(example_config
    idcu-common::idcu-common
    idcu-config::idcu-config
)

# Example 3: Metrics
add_executable(example_metrics example_metrics.c)
target_link_libraries(example_metrics
    idcu-common::idcu-common
    idcu-metrics::idcu-metrics
)

# Example 4: JSON
add_executable(example_json example_json.c)
target_link_libraries(example_json
    idcu-common::idcu-common
    idcu-json::idcu-json
)

# Example 5: YAML
add_executable(example_yaml example_yaml.c)
target_link_libraries(example_yaml
    idcu-common::idcu-common
    idcu-yaml::idcu-yaml
)

# Example 6: Network
add_executable(example_network example_network.c)
target_link_libraries(example_network
    idcu-common::idcu-common
    idcu-network::idcu-network
)

# Example 7: Module
add_executable(example_module example_module.c)
target_link_libraries(example_module
    idcu-common::idcu-common
    idcu-module-system::idcu-module-system
    idcu-log::idcu-log
)

# Example 8: Microkernel
add_executable(example_microkernel example_microkernel.c)
target_link_libraries(example_microkernel
    idcu-common::idcu-common
    idcu-microkernel::idcu-microkernel
    idcu-log::idcu-log
)

# Example 9: HTTP Server
add_executable(example_http_server example_http_server.c)
target_link_libraries(example_http_server
    idcu-common::idcu-common
    idcu-http-server::idcu-http-server
    idcu-log::idcu-log
)
