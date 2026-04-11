# idcu-management

Management library for IDCU Agent.

## Features
- Command line interface (CLI)
- HTTP management API
- Module management interface
- Configuration management interface
- System monitoring interface
- Thread-safe implementation
- CMake build configuration

## Usage

```c
#include <idcu/management/management.h>

idcu_Management_Context* ctx;
int ret = idcu_mgmt_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Execute management command
    idcu_MgmtResponse response;
    ret = idcu_mgmt_execute(ctx, IDCU_MGMT_CMD_STATUS, NULL, &response);
    if (ret == IDCU_ERR_OK) {
        // Handle response
        idcu_mgmt_response_destroy(&response);
    }
    
    // Shutdown
    idcu_mgmt_destroy(ctx);
}
```

## API Reference
- `idcu_mgmt_init()` - Initialize management library
- `idcu_mgmt_destroy()` - Shutdown management library
- `idcu_mgmt_execute()` - Execute management command
- `idcu_mgmt_response_destroy()` - Destroy response
- `idcu_mgmt_cli_start()` - Start CLI interface
- `idcu_mgmt_cli_stop()` - Stop CLI interface
- `idcu_mgmt_http_start()` - Start HTTP API
- `idcu_mgmt_http_stop()` - Stop HTTP API

## Building

```bash
cmake -B build && cmake --build build
```

## License
MIT
