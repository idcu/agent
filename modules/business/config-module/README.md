# idcu-config-module

Config business module for IDCU Agent, providing:
- Config loading and management
- Config hot reload
- Multi-environment config
- Config validation
- Config defaults
- Config backup
- Config rollback
- Config history
- Config change notification
- Config encryption

## Usage

```c
#include <idcu/config_module/config_module.h>

int main() {
    idcu_ConfigModuleConfig config;
    idcu_config_module_config_init(&config);
    
    idcu_ConfigModule cm;
    int ret = idcu_config_module_init(&cm, &config);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_config_module_start(&cm);
    if (ret != IDCU_ERR_OK) {
        idcu_config_module_destroy(&cm);
        return 1;
    }
    
    const char* value;
    idcu_config_module_get_string(&cm, "app.name", &value);
    printf("App name: %s\n", value);
    
    idcu_config_module_set_int(&cm, "app.port", 8080);
    idcu_config_module_save(&cm);
    
    idcu_config_module_stop(&cm);
    idcu_config_module_destroy(&cm);
    
    return 0;
}
```

## Configuration

The module can be configured via `config/app.yaml`:

```yaml
app:
  name: idcu-agent
  version: 1.0.0
  port: 8080
config:
  path: config/app.yaml
  hot_reload: false
  auto_backup: true
  encryption: false
  max_versions: 10
```
