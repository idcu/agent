# idcu-watchdog

IDCU Agent 的看门狗库。

## 功能特性
- 看门狗计时器
- 超时检测
- 自动恢复机制
- 线程安全实现

## 使用方法

```c
#include <idcu/watchdog/watchdog.h>

idcu_Watchdog* wd;
int ret = idcu_watchdog_init(&wd);
if (ret == IDCU_ERR_OK) {
    // 启动看门狗
    idcu_watchdog_start(wd, 5000); // 5秒超时
    
    // 定期喂狗
    idcu_watchdog_feed(wd);
    
    // 停止
    idcu_watchdog_stop(wd);
    
    // 清理
    idcu_watchdog_destroy(wd);
}
```

## API 参考
- `idcu_watchdog_init()` - 初始化看门狗
- `idcu_watchdog_destroy()` - 销毁看门狗
- `idcu_watchdog_is_initialized()` - 检查是否已初始化
- `idcu_watchdog_start()` - 启动看门狗
- `idcu_watchdog_stop()` - 停止看门狗
- `idcu_watchdog_feed()` - 喂狗
- `idcu_watchdog_get_operation_count()` - 获取操作计数
- `idcu_watchdog_get_error_count()` - 获取错误计数

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
