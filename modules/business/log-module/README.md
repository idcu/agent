# idcu-log-module

IDCU Agent 的日志业务模块，提供：
- 统一日志接口
- 多级日志（DEBUG、INFO、WARN、ERROR、FATAL）
- 多个输出目标（控制台、文件、消息总线）
- 日志轮转
- 日志过滤
- 日志查询
- 日志统计
- 日志告警

## 使用方法

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
    
    idcu_log_module_info(&lm, "应用程序已启动");
    idcu_log_module_debug(&lm, "调试消息");
    idcu_log_module_warn(&lm, "警告消息");
    idcu_log_module_error(&lm, "错误消息");
    
    idcu_log_module_stop(&lm);
    idcu_log_module_destroy(&lm);
    
    return 0;
}
```

## 配置

可以通过 `config/log.yaml` 配置模块：

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

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
