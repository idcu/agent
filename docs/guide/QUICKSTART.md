# Quick Start Guide

This guide will help you get started with IDCU Agent quickly.

## Prerequisites

### For All Platforms

- CMake 3.15 or later
- A C compiler:
  - GCC 9+ or Clang 11+ (Linux/macOS)
  - MSVC 2019+ (Windows)
- Git

### Additional Requirements (Optional)

- Docker (for containerized builds)
- Python 3.7+ (for some build scripts)

## Building the Project

### Clone the Repository

```bash
git clone <repository-url>
cd idcu-agent
```

### Linux/macOS Build

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### Windows Build (PowerShell)

```powershell
mkdir build ; cd build
cmake ..
cmake --build . --config Release
```

### Build Options

```bash
# Build with tests
cmake .. -DBUILD_TESTS=ON

# Build with examples
cmake .. -DBUILD_EXAMPLES=ON

# Release build
cmake .. -DCMAKE_BUILD_TYPE=Release
```

## Running Your First Program

### Hello World Example

Create a file `hello.c`:

```c
#include <idcu/log/log.h>
#include <idcu/common/error_code.h>

int main(void) {
    // Initialize logging
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    IDCU_LOG_INFO("Hello, IDCU Agent!");
    
    idcu_log_shutdown();
    return 0;
}
```

### Compile and Run

```bash
# Link with idcu-common and idcu-log
gcc hello.c -o hello -lidcu-common -lidcu-log
./hello
```

## Testing

### Running All Tests

```bash
cd build
ctest -V
```

### Running Specific Test

```bash
cd build
./bin/test_common
./bin/test_yaml
```

## Next Steps

- Read the [API Documentation](../api/README.md) for detailed library usage
- Explore the [Examples](../examples/) directory
- Check the [Architecture Guide](ARCHITECTURE.md) for system design

## Troubleshooting

### Build Errors

- Ensure CMake is in your PATH
- Check compiler version requirements
- Clean the build directory and retry:

```bash
rm -rf build
mkdir build && cd build
cmake ..
```

### Link Errors

- Ensure library search path includes the build output
- Specify libraries in correct order

## Getting Help

- Check the [README](../../README.md)
- Review the [Development Plan](../DEVELOPMENT_PLAN.md)
- Open an issue in the repository
