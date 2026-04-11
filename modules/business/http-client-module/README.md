# idcu-http-client-module

IDCU Agent 的 HTTP 客户端业务模块，提供：
- HTTP GET 请求
- HTTP POST 请求
- 连接池
- 超时处理
- SSL/TLS 支持

## 使用方法

```c
#include <idcu/http_client_module/http_client_module.h>

int main() {
    idcu_HttpClientModule hcm;
    int ret = idcu_http_client_module_init(&hcm);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_http_client_module_start(&hcm);
    if (ret != IDCU_ERR_OK) {
        idcu_http_client_module_destroy(&hcm);
        return 1;
    }
    
    char response[4096];
    size_t response_size;
    idcu_http_client_module_get(&hcm, "https://example.com", response, &response_size);
    
    idcu_http_client_module_stop(&hcm);
    idcu_http_client_module_destroy(&hcm);
    
    return 0;
}
```

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
