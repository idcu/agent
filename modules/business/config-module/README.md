# idcu-config-module

IDCU Agent 的配置业务模块，提供：
- 配置加载和管理
- 配置热重载
- 多环境配置
- 配置验证
- 配置默认值
- 配置备份
- 配置回滚
- 配置历史
- 配置变更通知
- 配置加密

## 使用方法

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
    printf("应用名称: %s\n", value);
    
    idcu_config_module_set_int(&cm, "app.port", 8080);
    idcu_config_module_save(&cm);
    
    idcu_config_module_stop(&cm);
    idcu_config_module_destroy(&cm);
    
    return 0;
}
```

## 配置

可以通过 `config/app.yaml` 配置模块：

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

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
