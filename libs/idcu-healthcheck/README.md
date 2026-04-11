# idcu-healthcheck

IDCU Agent 的健康检查库。

## 功能特性
- 多种健康检查类型（HTTP、TCP、数据库、自定义）
- 检查结果缓存
- 健康状态变更通知
- 超时处理
- 线程安全实现

## 使用方法

```c
#include <idcu/healthcheck/healthcheck.h>

idcu_Healthcheck* hc;
int ret = idcu_healthcheck_init(&hc);
if (ret == IDCU_ERR_OK) {
    // 添加 HTTP 检查
    idcu_healthcheck_add_http(hc, "api", "http://localhost:8080/health", 5000);
    
    // 添加 TCP 检查
    idcu_healthcheck_add_tcp(hc, "database", "localhost", 5432, 3000);
    
    // 执行检查
    idcu_HealthcheckResult result;
    idcu_healthcheck_check(hc, &result);
    
    // 清理
    idcu_healthcheck_destroy(hc);
}
```

## API 参考
- `idcu_healthcheck_init()` - 初始化健康检查
- `idcu_healthcheck_destroy()` - 销毁健康检查
- `idcu_healthcheck_is_initialized()` - 检查是否已初始化
- `idcu_healthcheck_add_http()` - 添加 HTTP 检查
- `idcu_healthcheck_add_tcp()` - 添加 TCP 检查
- `idcu_healthcheck_add_custom()` - 添加自定义检查
- `idcu_healthcheck_check()` - 执行健康检查
- `idcu_healthcheck_get_status()` - 获取健康状态

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
