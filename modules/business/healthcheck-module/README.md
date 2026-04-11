# idcu-healthcheck-module

IDCU Agent 的健康检查业务模块，提供：
- 周期性健康检查
- 自定义健康检查
- 健康状态聚合
- 健康状态通知
- HTTP API 查询

## 使用方法

```c
#include <idcu/healthcheck_module/healthcheck_module.h>

int main() {
    idcu_HealthCheckModule hcm;
    int ret = idcu_healthcheck_module_init(&hcm);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_healthcheck_module_start(&hcm);
    if (ret != IDCU_ERR_OK) {
        idcu_healthcheck_module_destroy(&hcm);
        return 1;
    }
    
    idcu_HealthCheckResult result;
    idcu_healthcheck_module_get_status(&hcm, &result);
    printf("健康状态: %d\n", result.status);
    
    idcu_healthcheck_module_stop(&hcm);
    idcu_healthcheck_module_destroy(&hcm);
    
    return 0;
}
```

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
