# idcu-alert-module

IDCU Agent 的告警业务模块，提供：
- 告警规则配置
- 告警触发和去重
- 多渠道告警通知（消息总线、HTTP 等）
- 告警历史记录
- 告警查询和管理

## 使用方法

```c
#include <idcu/alert_module/alert_module.h>

int main() {
    idcu_AlertModule am;
    int ret = idcu_alert_module_init(&am);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_alert_module_start(&am);
    if (ret != IDCU_ERR_OK) {
        idcu_alert_module_destroy(&am);
        return 1;
    }
    
    idcu_alert_module_trigger(&am, "high_cpu_usage", NULL);
    
    idcu_alert_module_stop(&am);
    idcu_alert_module_destroy(&am);
    
    return 0;
}
```

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
