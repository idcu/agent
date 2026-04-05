# HTTP Client 业务模块

## 功能说明

HTTP 客户端业务模块，提供 HTTP/REST API 调用功能。

## 依赖关系

- 基础公共库
- 工具库
- 微内核
- 网络服务模块

## 主要功能

- 支持 HTTP GET/POST/PUT/DELETE/PATCH/HEAD/OPTIONS 方法
- URL 解析
- 请求头管理
- 响应解析
- 便捷的 GET/POST 接口

## 使用示例

```c
// 简单的 GET 请求
char response[4096];
size_t len = sizeof(response);
int status = idcu_http_client_module_get("http://example.com/api", response, &len);
if (status == 200) {
    printf("Response: %s\n", response);
}

// 简单的 POST 请求
const char* post_data = "{\"key\": \"value\"}";
len = sizeof(response);
status = idcu_http_client_module_post("http://example.com/api", post_data, strlen(post_data), response, &len);
```
