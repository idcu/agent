# idcu-alert

IDCU Agent 的告警库。

## 功能特性
- 多种告警级别
- 告警通知机制
- 告警历史记录
- 线程安全实现

## 使用方法

```c
#include <idcu/alert/alert.h>

idcu_Alert* alert;
int ret = idcu_alert_init(&alert);
if (ret == IDCU_ERR_OK) {
    // 发送告警
    idcu_alert_send(alert, IDCU_ALERT_LEVEL_WARNING, "系统告警", "内存使用率过高");
    
    // 获取告警历史
    idcu_AlertHistory* history;
    idcu_alert_get_history(alert, &history);
    
    // 清理
    idcu_alert_destroy(alert);
}
```

## API 参考
- `idcu_alert_init()` - 初始化告警系统
- `idcu_alert_destroy()` - 销毁告警系统
- `idcu_alert_is_initialized()` - 检查是否已初始化
- `idcu_alert_send()` - 发送告警
- `idcu_alert_get_history()` - 获取告警历史
- `idcu_alert_get_operation_count()` - 获取操作计数
- `idcu_alert_get_error_count()` - 获取错误计数

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
