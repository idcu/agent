# idcu-rest-api

IDCU Agent 的 REST API 模块，提供完整的 HTTP API 功能。

## 功能特性

- HTTP API 端点管理
- 请求路由和分发
- 多种认证方式（Basic、Bearer、API Key）
- API 版本控制
- 请求/响应日志
- 限流控制（令牌桶算法）
- CORS 支持
- OpenAPI 文档生成

## 使用方法

```c
#include <idcu/rest_api/rest_api.h>
#include <stdio.h>

void status_handler(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    (void)user_data;
    
    const char* json = 
        "{\n"
        "  \"status\": \"ok\",\n"
        "  \"version\": \"1.0.0\"\n"
        "}";
    
    idcu_rest_api_response_json(response, json);
}

int main(void) {
    printf("=== REST API Server ===\n\n");

    idcu_RestApi* api = NULL;
    int ret = idcu_rest_api_init(&api);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to initialize REST API\n");
        return 1;
    }

    idcu_rest_api_get(api, "/api/v1/status", status_handler, NULL);

    idcu_ApiCorsConfig cors_config = {0};
    cors_config.enabled = true;
    strncpy(cors_config.allowed_origins, "*", sizeof(cors_config.allowed_origins) - 1);
    idcu_rest_api_set_cors(api, &cors_config);

    printf("Starting server on 0.0.0.0:8080...\n");
    idcu_rest_api_listen(api, "0.0.0.0", 8080, 10);

    idcu_rest_api_destroy(api);
    return 0;
}
```

## API 参考

- `idcu_rest_api_init()` - 初始化 REST API
- `idcu_rest_api_destroy()` - 销毁 REST API
- `idcu_rest_api_listen()` - 启动服务器监听
- `idcu_rest_api_stop()` - 停止服务器
- `idcu_rest_api_add_endpoint()` - 添加 API 端点
- `idcu_rest_api_get()` - 添加 GET 端点
- `idcu_rest_api_post()` - 添加 POST 端点
- `idcu_rest_api_put()` - 添加 PUT 端点
- `idcu_rest_api_delete()` - 添加 DELETE 端点
- `idcu_rest_api_set_auth()` - 设置认证配置
- `idcu_rest_api_set_rate_limit()` - 设置限流配置
- `idcu_rest_api_set_cors()` - 设置 CORS 配置
- `idcu_rest_api_generate_openapi()` - 生成 OpenAPI 文档
- `idcu_rest_api_response_json()` - 发送 JSON 响应
- `idcu_rest_api_response_error()` - 发送错误响应
- `idcu_rest_api_response_success()` - 发送成功响应

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
