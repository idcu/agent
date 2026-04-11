# idcu-log-module

Log business module for IDCU Agent, providing:
- Unified log interface
- Multi-level logging (DEBUG, INFO, WARN, ERROR, FATAL)
- Multiple output targets (console, file, message bus)
- Log rotation
- Log filtering
- Log query
- Log stats
- Log alert

## Usage

```c
#include <idcu/log_module/log_module.h>

int main() {
    idcu_LogModuleConfig config;
    idcu_log_module_config_init(&config);
    
    config.outputs[IDCU_LOG_OUTPUT_FILE].enabled = 1;
    
    idcu_LogModule lm;
    int ret = idcu_log_module_init(&lm, &config);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_log_module_start(&lm);
    if (ret != IDCU_ERR_OK) {
        idcu_log_module_destroy(&lm);
        return 1;
    }
    
    idcu_log_module_info(&lm, "Application started");
    idcu_log_module_debug(&lm, "Debug message");
    idcu_log_module_warn(&lm, "Warning message");
    idcu_log_module_error(&lm, "Error message");
    
    idcu_log_module_stop(&lm);
    idcu_log_module_destroy(&lm);
    
    return 0;
}
```

## Configuration

The module can be configured via `config/log.yaml`:

```yaml
log:
  level: info
  outputs:
    console:
      enabled: true
      min_level: debug
    file:
      enabled: true
      path: logs/app.log
      min_level: info
    msgbus:
      enabled: false
  rotation:
    enabled: true
    max_file_size: 104857600
    max_file_count: 10
```
