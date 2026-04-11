# idcu-core-module

Core foundation module for IDCU Agent, providing:
- Module initialization and cleanup
- Config loading and management
- Logging initialization
- Module lifecycle management
- Health check integration
- Metrics collection integration
- Graceful shutdown
- Signal handling

## Usage

```c
#include <idcu/core_module/core_module.h>

int main() {
    idcu_CoreModuleConfig config;
    idcu_core_module_config_init(&config);
    
    idcu_CoreModule module;
    int ret = idcu_core_module_init(&module, &config);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_core_module_start(&module);
    if (ret != IDCU_ERR_OK) {
        idcu_core_module_destroy(&module);
        return 1;
    }
    
    while (!idcu_core_module_should_shutdown(&module)) {
    }
    
    idcu_core_module_stop(&module);
    idcu_core_module_destroy(&module);
    
    return 0;
}
```

## Configuration

The module can be configured via `config/app.yaml`:

```yaml
core:
  name: idcu-agent
  log:
    level: info
    path: logs/idcu.log
  healthcheck:
    enabled: true
    interval: 5000
  metrics:
    enabled: true
  graceful_shutdown_timeout: 30000
```
