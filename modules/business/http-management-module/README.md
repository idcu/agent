# idcu-http-management-module

IDCU Agent 的 HTTP 管理业务模块，提供：
- 用于管理 API 的 HTTP 服务器
- RESTful API 端点
- 系统健康监控 API
- 配置管理 API
- 模块管理 API

## 使用方法

```c
#include <idcu/http_management_module/http_management_module.h>

int main() {
    idcu_HttpManagementModule hmm;
    int ret = idcu_http_management_module_init(&hmm);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_http_management_module_start(&hmm);
    if (ret != IDCU_ERR_OK) {
        idcu_http_management_module_destroy(&hmm);
        return 1;
    }
    
    idcu_http_management_module_stop(&hmm);
    idcu_http_management_module_destroy(&hmm);
    
    return 0;
}
```

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
