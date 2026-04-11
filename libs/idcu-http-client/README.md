# idcu-http-client

IDCU Agent 的 HTTP 客户端库。

## 功能特性
- 简单的 HTTP 客户端
- 支持 GET、POST、PUT、DELETE 等方法
- 请求和响应处理
- 超时设置
- 线程安全实现

## 使用方法

```c
#include <idcu/http-client/http-client.h>

idcu_HttpClient* client;
int ret = idcu_http_client_init(&client);
if (ret == IDCU_ERR_OK) {
    // 发送 GET 请求
    idcu_HttpResponse* response;
    ret = idcu_http_client_get(client, "http://example.com", &response);
    if (ret == IDCU_ERR_OK) {
        printf("Status: %d\n", response->status_code);
        printf("Body: %s\n", response->body);
        idcu_http_response_destroy(response);
    }
    
    // 发送 POST 请求
    const char* data = "{\"key\": \"value\"}";
    ret = idcu_http_client_post(client, "http://example.com/api", data, strlen(data), &response);
    
    // 清理
    idcu_http_client_destroy(client);
}
```

## API 参考
- `idcu_http_client_init()` - 初始化 HTTP 客户端
- `idcu_http_client_destroy()` - 销毁 HTTP 客户端
- `idcu_http_client_is_initialized()` - 检查是否已初始化
- `idcu_http_client_get()` - 发送 GET 请求
- `idcu_http_client_post()` - 发送 POST 请求
- `idcu_http_client_put()` - 发送 PUT 请求
- `idcu_http_client_delete()` - 发送 DELETE 请求
- `idcu_http_response_destroy()` - 销毁响应

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
