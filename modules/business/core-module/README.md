# idcu-core-module

IDCU Agent 的核心基础模块，提供：
- 模块初始化和清理
- 配置加载和管理
- 日志初始化
- 模块生命周期管理
- 健康检查集成
- 指标收集集成
- 优雅关闭
- 信号处理

## 使用方法

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

## 配置

可以通过 `config/app.yaml` 配置模块：

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

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
