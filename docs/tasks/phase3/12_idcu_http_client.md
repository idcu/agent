# 任务 3.12: idcu-http-client - HTTP 客户端库

## 目标

创建轻量级 HTTP 客户端库，支持：
- HTTP/1.1 协议
- GET/POST/PUT/DELETE 等方法
- 请求头设置
- 查询参数
- 请求体（表单、JSON）
- 响应解析
- 超时设置
- 代理支持

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-http-client/include/idcu/http
mkdir -p libs/idcu-http-client/src/idcu/http
mkdir -p libs/idcu-http-client/tests
mkdir -p libs/idcu-http-client/examples
```

### 2. 创建 HTTP 客户端头文件 (http_client.h)

创建 `libs/idcu-http-client/include/idcu/http/http_client.h`：

```c
#ifndef IDCU_HTTP_HTTP_CLIENT_H
#define IDCU_HTTP_HTTP_CLIENT_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/network/network.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_HTTP_CLIENT_METHOD_GET = 0,
    IDCU_HTTP_CLIENT_METHOD_POST,
    IDCU_HTTP_CLIENT_METHOD_PUT,
    IDCU_HTTP_CLIENT_METHOD_DELETE,
    IDCU_HTTP_CLIENT_METHOD_HEAD,
    IDCU_HTTP_CLIENT_METHOD_OPTIONS,
    IDCU_HTTP_CLIENT_METHOD_PATCH
} idcu_HttpClientMethod;

typedef struct
{
    char key[256];
    char value[1024];
} idcu_HttpClientHeader;

typedef struct
{
    idcu_HttpClientHeader headers[64];
    size_t count;
} idcu_HttpClientHeaders;

typedef struct
{
    idcu_HttpClientMethod method;
    char url[2048];
    idcu_HttpClientHeaders headers;
    char* body;
    size_t body_length;
    int timeout_ms;
    int follow_redirects;
    int max_redirects;
    char proxy_host[256];
    uint16_t proxy_port;
    int use_proxy;
} idcu_HttpClientRequest;

typedef struct
{
    int status_code;
    char reason[256];
    char version[32];
    idcu_HttpClientHeaders headers;
    char* body;
    size_t body_length;
    size_t body_capacity;
    char final_url[2048];
    int redirect_count;
} idcu_HttpClientResponse;

typedef struct
{
    idcu_TcpSocket socket;
    int connected;
    int timeout_ms;
    char host[256];
    uint16_t port;
    int use_ssl;
} idcu_HttpClient;

int  idcu_http_client_init(idcu_HttpClient* client);
void idcu_http_client_destroy(idcu_HttpClient* client);
int  idcu_http_client_connect(idcu_HttpClient* client, const char* host, uint16_t port);
int  idcu_http_client_connect_timeout(idcu_HttpClient* client, const char* host, uint16_t port, int timeout_ms);
void idcu_http_client_disconnect(idcu_HttpClient* client);
int  idcu_http_client_set_timeout(idcu_HttpClient* client, int timeout_ms);

int  idcu_http_client_request_init(idcu_HttpClientRequest* request);
void idcu_http_client_request_destroy(idcu_HttpClientRequest* request);
int  idcu_http_client_request_set_method(idcu_HttpClientRequest* request, idcu_HttpClientMethod method);
int  idcu_http_client_request_set_url(idcu_HttpClientRequest* request, const char* url);
int  idcu_http_client_request_set_header(idcu_HttpClientRequest* request, const char* name, const char* value);
int  idcu_http_client_request_set_body(idcu_HttpClientRequest* request, const void* data, size_t length);
int  idcu_http_client_request_set_json(idcu_HttpClientRequest* request, const char* json);
int  idcu_http_client_request_set_form(idcu_HttpClientRequest* request, const char* form_data);
int  idcu_http_client_request_add_query_param(idcu_HttpClientRequest* request, const char* name, const char* value);
int  idcu_http_client_request_set_timeout(idcu_HttpClientRequest* request, int timeout_ms);
int  idcu_http_client_request_set_follow_redirects(idcu_HttpClientRequest* request, int follow, int max_redirects);
int  idcu_http_client_request_set_proxy(idcu_HttpClientRequest* request, const char* host, uint16_t port);
const char* idcu_http_client_method_to_string(idcu_HttpClientMethod method);

int  idcu_http_client_response_init(idcu_HttpClientResponse* response);
void idcu_http_client_response_destroy(idcu_HttpClientResponse* response);
int  idcu_http_client_response_get_header(const idcu_HttpClientResponse* response, const char* name, const char** value);
int  idcu_http_client_response_get_status_code(const idcu_HttpClientResponse* response);
const char* idcu_http_client_response_get_body(const idcu_HttpClientResponse* response);
size_t idcu_http_client_response_get_body_length(const idcu_HttpClientResponse* response);

int idcu_http_client_execute(idcu_HttpClient* client, const idcu_HttpClientRequest* request, idcu_HttpClientResponse* response);
int idcu_http_client_get(idcu_HttpClient* client, const char* url, idcu_HttpClientResponse* response);
int idcu_http_client_post(idcu_HttpClient* client, const char* url, const void* data, size_t length, idcu_HttpClientResponse* response);
int idcu_http_client_post_json(idcu_HttpClient* client, const char* url, const char* json, idcu_HttpClientResponse* response);
int idcu_http_client_put(idcu_HttpClient* client, const char* url, const void* data, size_t length, idcu_HttpClientResponse* response);
int idcu_http_client_delete(idcu_HttpClient* client, const char* url, idcu_HttpClientResponse* response);

int idcu_http_client_headers_init(idcu_HttpClientHeaders* headers);
int idcu_http_client_headers_add(idcu_HttpClientHeaders* headers, const char* name, const char* value);
int idcu_http_client_headers_get(const idcu_HttpClientHeaders* headers, const char* name, const char** value);
void idcu_http_client_headers_clear(idcu_HttpClientHeaders* headers);

int idcu_http_url_parse(const char* url, char* host, size_t host_size, uint16_t* port, char* path, size_t path_size, char* query, size_t query_size);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-http-client/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-http-client VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-http-client STATIC
    src/idcu/http/http_client.c
)

target_include_directories(idcu-http-client PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-http-client PRIVATE
    idcu::common
    idcu::network
    idcu::log
)

add_library(idcu::http-client ALIAS idcu-http-client)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-http-client/module.yaml`：

```yaml
name: idcu-http-client
version: 1.0.0
description: HTTP client library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-network
  - idcu-log

build:
  type: cmake
  targets:
    - idcu-http-client

headers:
  - idcu/http/http_client.h

features:
  - http11: HTTP/1.1 protocol support
  - methods: GET/POST/PUT/DELETE/HEAD/OPTIONS/PATCH
  - headers: HTTP header support
  - query: Query parameter support
  - body: Request body support (form, JSON)
  - response: Response parsing
  - timeout: Timeout settings
  - redirect: Follow redirects support
  - proxy: Proxy support

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `libs/idcu-http-client/README.md`：

```markdown
# idcu-http-client

IDCU Agent 的轻量级 HTTP 客户端库。

## 功能特性

- **HTTP/1.1**: HTTP/1.1 协议支持
- **HTTP 方法**: GET/POST/PUT/DELETE/HEAD/OPTIONS/PATCH
- **请求头**: HTTP 头支持
- **查询参数**: 查询参数支持
- **请求体**: 请求体支持（表单、JSON）
- **响应解析**: 响应解析
- **超时设置**: 超时设置
- **重定向**: 重定向跟随支持
- **代理**: 代理支持

## 快速开始

### 初始化客户端

```c
#include "idcu/http/http_client.h"

idcu_HttpClient client;
idcu_http_client_init(&client);
```

### 简单 GET 请求

```c
idcu_HttpClientResponse response;
idcu_http_client_response_init(&response);

int ret = idcu_http_client_get(&client, "https://api.example.com/data", &response);
if (ret == IDCU_ERR_OK) {
    printf("Status: %d\n", response.status_code);
    printf("Body: %.*s\n", (int)response.body_length, response.body);
}

idcu_http_client_response_destroy(&response);
```

### POST JSON 数据

```c
idcu_HttpClientRequest request;
idcu_http_client_request_init(&request);

idcu_http_client_request_set_method(&request, IDCU_HTTP_CLIENT_METHOD_POST);
idcu_http_client_request_set_url(&request, "https://api.example.com/users");
idcu_http_client_request_set_header(&request, "Content-Type", "application/json");
idcu_http_client_request_set_json(&request, "{\"name\": \"John\", \"age\": 30}");

idcu_HttpClientResponse response;
idcu_http_client_response_init(&response);

idcu_http_client_execute(&client, &request, &response);

idcu_http_client_request_destroy(&request);
idcu_http_client_response_destroy(&response);
```

### 使用查询参数

```c
idcu_HttpClientRequest request;
idcu_http_client_request_init(&request);

idcu_http_client_request_set_method(&request, IDCU_HTTP_CLIENT_METHOD_GET);
idcu_http_client_request_set_url(&request, "https://api.example.com/search");
idcu_http_client_request_add_query_param(&request, "q", "hello");
idcu_http_client_request_add_query_param(&request, "page", "1");

idcu_HttpClientResponse response;
idcu_http_client_response_init(&response);

idcu_http_client_execute(&client, &request, &response);

idcu_http_client_request_destroy(&request);
idcu_http_client_response_destroy(&response);
```

### 设置超时

```c
idcu_http_client_set_timeout(&client, 5000);
```

### 跟随重定向

```c
idcu_HttpClientRequest request;
idcu_http_client_request_init(&request);

idcu_http_client_request_set_url(&request, "https://example.com");
idcu_http_client_request_set_follow_redirects(&request, 1, 5);
```

### 使用代理

```c
idcu_HttpClientRequest request;
idcu_http_client_request_init(&request);

idcu_http_client_request_set_url(&request, "https://example.com");
idcu_http_client_request_set_proxy(&request, "proxy.example.com", 8080);
```

### 获取响应头

```c
const char* content_type = NULL;
idcu_http_client_response_get_header(&response, "Content-Type", &content_type);
if (content_type) {
    printf("Content-Type: %s\n", content_type);
}
```

### 清理

```c
idcu_http_client_destroy(&client);
```

## HTTP 方法

| 方法 | 说明 |
|-----|------|
| GET | 获取资源 |
| POST | 创建资源 |
| PUT | 更新资源 |
| DELETE | 删除资源 |
| HEAD | 获取头部 |
| OPTIONS | 获取允许的方法 |
| PATCH | 部分更新 |

## API 文档

详见 [include/idcu/http/http_client.h](include/idcu/http/http_client.h)
```

## 验证检查清单

- [ ] HTTP 客户端头文件已创建
- [ ] HTTP 客户端实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以发送 GET 请求
- [ ] 可以发送 POST 请求
- [ ] 可以解析响应

## Git 提交

```bash
git add libs/idcu-http-client/
git commit -m "feat: add idcu-http-client library

- Add HTTP/1.1 protocol support
- Add GET/POST/PUT/DELETE/HEAD/OPTIONS/PATCH methods
- Add HTTP header support
- Add query parameter support
- Add request body support (form, JSON)
- Add response parsing
- Add timeout settings
- Add follow redirects support
- Add proxy support
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 连接超时 | 网络问题或服务器无响应 | 增加超时时间或检查网络 |
| SSL 错误 | 未配置 SSL | 使用 HTTPS 时需要 SSL 支持 |
| 重定向循环 | max_redirects 太小 | 增加 max_redirects 值 |
