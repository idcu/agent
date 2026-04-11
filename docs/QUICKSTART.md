# IDCU Agent Quick Start Guide

This guide will help you get started with the IDCU Agent project quickly.

## Prerequisites

- CMake 3.15 or higher
- A C compiler (GCC, Clang, or MSVC)
- Git (optional, for cloning the repository)

## Building the Project

### Step 1: Clone or Download the Project

```bash
git clone <repository-url>
cd idcu-agent
```

### Step 2: Create Build Directory

```bash
mkdir build
cd build
```

### Step 3: Configure with CMake

#### Windows (MSVC)
```bash
cmake .. -G "Visual Studio 17 2022" -A x64
```

#### Windows (MinGW)
```bash
cmake .. -G "MinGW Makefiles"
```

#### Linux (GCC)
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

#### Linux (Clang)
```bash
cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Release
```

### Step 4: Build

```bash
# Windows (Visual Studio)
cmake --build . --config Release

# Linux/Mac (Make)
make -j4
```

### Step 5: Run the Application

```bash
# After successful build
./app/idcu-agent --version
```

## Project Structure

```
idcu-agent/
├── app/                    # Main application
├── libs/                   # Core libraries
│   ├── idcu-common/       # Common utilities
│   ├── idcu-log/          # Logging system
│   ├── idcu-module-system/# Module system
│   ├── idcu-coroutine/    # Coroutine scheduler
│   ├── idcu-msgbus/       # Message bus
│   ├── idcu-microkernel/  # Microkernel
│   ├── idcu-network/      # Network library
│   ├── idcu-http-server/  # HTTP server
│   ├── idcu-http-client/  # HTTP client
│   ├── idcu-config/       # Configuration
│   ├── idcu-metrics/      # Metrics
│   ├── idcu-healthcheck/  # Health checks
│   └── ...                # More libraries
├── modules/                # Business modules
├── tests/                  # Tests
├── docs/                   # Documentation
└── CMakeLists.txt          # Main build file
```

## First Program: Using the Logging Library

Create a file `hello_log.c`:

```c
#include <idcu/log/log.h>

int main() {
    // Initialize logging
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    IDCU_LOG_INFO("Hello, IDCU Agent!");
    IDCU_LOG_WARN("This is a warning");
    
    // Cleanup
    idcu_log_shutdown();
    
    return 0;
}
```

## Building Your Own Program

Create a `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.15)
project(my_program)

# Find or include IDCU libraries
add_executable(my_program hello_log.c)
target_link_libraries(my_program PRIVATE idcu::log)
```

## Using Configuration

```c
#include <idcu/config/config.h>

int main() {
    idcu_Config* config = NULL;
    idcu_config_init(&config, "app.ini");
    
    const char* host = idcu_config_get_string(config, "database", "host", "localhost");
    int port = idcu_config_get_int(config, "database", "port", 5432);
    
    printf("Connecting to %s:%d\n", host, port);
    
    idcu_config_destroy(config);
    return 0;
}
```

## Using Metrics

```c
#include <idcu/metrics/metrics.h>

int main() {
    idcu_MetricsRegistry* registry = NULL;
    idcu_metrics_registry_init(&registry);
    
    // Create a counter
    idcu_Metric* requests = idcu_metrics_counter_create(
        registry, "http_requests_total", "Total HTTP requests"
    );
    
    idcu_metrics_counter_inc(requests);
    
    // Export to Prometheus format
    char* prom = idcu_metrics_export_prometheus(registry);
    printf("%s", prom);
    free(prom);
    
    idcu_metrics_registry_destroy(registry);
    return 0;
}
```

## Running Tests

To build and run tests:

```bash
cd build
cmake .. -DBUILD_TESTS=ON
cmake --build .
ctest --output-on-failure
```

## Next Steps

- Read the [API Documentation](api/README.md) for detailed library usage
- Check the [examples](examples/) directory for more code examples
- See the [Development Plan](DEVELOPMENT_PLAN.md) for project roadmap
- Review the [Tasks](tasks/README.md) for contribution guidelines

## Getting Help

- Check the documentation in the `docs/` directory
- Look at the header files in `libs/*/include/` for API definitions
- Review the source code in `libs/*/src/` for implementation details

## License

IDCU Agent is released under the MIT License.
