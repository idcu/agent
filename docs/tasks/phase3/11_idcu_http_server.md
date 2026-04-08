# 任务 3.11: idcu-http-server - HTTP 服务器库

## 目标

创建轻量级 HTTP 服务器库，支持：
- HTTP/1.1 协议
- 路由处理
- GET/POST/PUT/DELETE 方法
- 请求和响应处理
- 查询参数解析
- 表单数据解析
- JSON 请求/响应
- 静态文件服务

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-http-server/include/idcu/http
mkdir -p libs/idcu-http-server/src/idcu/http
mkdir -p libs/idcu-http-server/tests
mkdir -p libs/idcu-http-server/examples
```

### 2. 创建 HTTP 服务器头文件 (http_server.h)

创建 `libs/idcu-http-server/include/idcu/http/http_server.h`：

```c
#ifndef IDCU_HTTP_HTTP_SERVER_H
#define IDCU_HTTP_HTTP_SERVER_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include "idcu/network/network.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_HTTP_METHOD_GET = 0,
    IDCU_HTTP_METHOD_POST,
    IDCU_HTTP_METHOD_PUT,
    IDCU_HTTP_METHOD_DELETE,
    IDCU_HTTP_METHOD_HEAD,
    IDCU_HTTP_METHOD_OPTIONS,
    IDCU_HTTP_METHOD_PATCH
} idcu_HttpMethod;

typedef enum
{
    IDCU_HTTP_STATUS_OK = 200,
    IDCU_HTTP_STATUS_CREATED = 201,
    IDCU_HTTP_STATUS_ACCEPTED = 202,
    IDCU_HTTP_STATUS_NO_CONTENT = 204,
    IDCU_HTTP_STATUS_BAD_REQUEST = 400,
    IDCU_HTTP_STATUS_UNAUTHORIZED = 401,
    IDCU_HTTP_STATUS_FORBIDDEN = 403,
    IDCU_HTTP_STATUS_NOT_FOUND = 404,
    IDCU_HTTP_STATUS_METHOD_NOT_ALLOWED = 405,
    IDCU_HTTP_STATUS_INTERNAL_SERVER_ERROR = 500,
    IDCU_HTTP_STATUS_NOT_IMPLEMENTED = 501,
    IDCU_HTTP_STATUS_SERVICE_UNAVAILABLE = 503
} idcu_HttpStatus;

typedef struct
{
    char key[256];
    char value[1024];
} idcu_HttpHeader;

typedef struct
{
    idcu_HttpHeader headers[64];
    size_t count;
} idcu_HttpHeaders;

typedef struct
{
    char key[128];
    char value[512];
} idcu_HttpQueryParam;

typedef struct
{
    idcu_HttpQueryParam params[32];
    size_t count;
} idcu_HttpQueryParams;

typedef struct
{
    idcu_HttpMethod method;
    char path[1024];
    char version[32];
    idcu_HttpHeaders headers;
    idcu_HttpQueryParams query_params;
    char* body;
    size_t body_length;
    char client_ip[64];
    uint16_t client_port;
} idcu_HttpRequest;

typedef struct
{
    idcu_HttpStatus status;
    char reason[256];
    char version[32];
    idcu_HttpHeaders headers;
    char* body;
    size_t body_length;
    size_t body_capacity;
} idcu_HttpResponse;

typedef struct idcu_HttpRoute idcu_HttpRoute;

typedef void (*idcu_HttpHandler)(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data);

struct idcu_HttpRoute
{
    char path[1024];
    idcu_HttpMethod method;
    idcu_HttpHandler handler;
    void* user_data;
};

typedef struct
{
    idcu_Vector routes;
    idcu_TcpServer server;
    idcu_Mutex lock;
    int running;
    int initialized;
    char static_dir[1024];
    int serve_static;
} idcu_HttpServer;

int  idcu_http_server_init(idcu_HttpServer* server);
void idcu_http_server_destroy(idcu_HttpServer* server);
int  idcu_http_server_listen(idcu_HttpServer* server, const char* host, uint16_t port, int backlog);
void idcu_http_server_stop(idcu_HttpServer* server);
int  idcu_http_server_set_static_dir(idcu_HttpServer* server, const char* dir);

int idcu_http_server_add_route(idcu_HttpServer* server, idcu_HttpMethod method, const char* path, idcu_HttpHandler handler, void* user_data);
int idcu_http_server_get(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data);
int idcu_http_server_post(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data);
int idcu_http_server_put(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data);
int idcu_http_server_delete(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data);

int  idcu_http_request_init(idcu_HttpRequest* request);
void idcu_http_request_destroy(idcu_HttpRequest* request);
int  idcu_http_request_get_header(const idcu_HttpRequest* request, const char* name, const char** value);
int  idcu_http_request_get_query_param(const idcu_HttpRequest* request, const char* name, const char** value);
const char* idcu_http_method_to_string(idcu_HttpMethod method);

int  idcu_http_response_init(idcu_HttpResponse* response);
void idcu_http_response_destroy(idcu_HttpResponse* response);
void idcu_http_response_set_status(idcu_HttpResponse* response, idcu_HttpStatus status);
int  idcu_http_response_set_header(idcu_HttpResponse* response, const char* name, const char* value);
int  idcu_http_response_set_body(idcu_HttpResponse* response, const void* data, size_t length);
int  idcu_http_response_append_body(idcu_HttpResponse* response, const void* data, size_t length);
int  idcu_http_response_set_string(idcu_HttpResponse* response, const char* str);
int  idcu_http_response_set_json(idcu_HttpResponse* response, const char* json);
void idcu_http_response_send_error(idcu_HttpResponse* response, idcu_HttpStatus status, const char* message);
const char* idcu_http_status_to_string(idcu_HttpStatus status);

int idcu_http_headers_init(idcu_HttpHeaders* headers);
int idcu_http_headers_add(idcu_HttpHeaders* headers, const char* name, const char* value);
int idcu_http_headers_get(const idcu_HttpHeaders* headers, const char* name, const char** value);
void idcu_http_headers_clear(idcu_HttpHeaders* headers);

int idcu_http_query_params_init(idcu_HttpQueryParams* params);
int idcu_http_query_params_parse(idcu_HttpQueryParams* params, const char* query_string);
int idcu_http_query_params_get(const idcu_HttpQueryParams* params, const char* name, const char** value);
void idcu_http_query_params_clear(idcu_HttpQueryParams* params);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-http-server/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-http-server VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-http-server STATIC
    src/idcu/http/http_server.c
)

target_include_directories(idcu-http-server PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-http-server PRIVATE
    idcu::common
    idcu::network
    idcu::log
)

add_library(idcu::http-server ALIAS idcu-http-server)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-http-server/module.yaml`：

```yaml
name: idcu-http-server
version: 1.0.0
description: HTTP server library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-network
  - idcu-log

build:
  type: cmake
  targets:
    - idcu-http-server

headers:
  - idcu/http/http_server.h

features:
  - http11: HTTP/1.1 protocol support
  - routing: Route-based request handling
  - methods: GET/POST/PUT/DELETE/HEAD/OPTIONS/PATCH
  - headers: HTTP header parsing and setting
  - query: Query parameter parsing
  - json: JSON request/response support
  - static: Static file serving
  - forms: Form data parsing

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `libs/idcu-http-server/README.md`：

```markdown
# idcu-http-server

IDCU Agent 的轻量级 HTTP 服务器库。

## 功能特性

- **HTTP/1.1**: HTTP/1.1 协议支持
- **路由处理**: 基于路由的请求处理
- **HTTP 方法**: GET/POST/PUT/DELETE/HEAD/OPTIONS/PATCH
- **请求头**: HTTP 头解析和设置
- **查询参数**: 查询参数解析
- **JSON 支持**: JSON 请求/响应支持
- **静态文件**: 静态文件服务
- **表单数据**: 表单数据解析

## 快速开始

### 初始化服务器

```c
#include "idcu/http/http_server.h"

idcu_HttpServer server;
idcu_http_server_init(&server);
```

### 定义路由处理器

```c
void hello_handler(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data)
{
    idcu_http_response_set_status(response, IDCU_HTTP_STATUS_OK);
    idcu_http_response_set_header(response, "Content-Type", "text/plain");
    idcu_http_response_set_string(response, "Hello, World!");
}

void json_handler(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data)
{
    idcu_http_response_set_status(response, IDCU_HTTP_STATUS_OK);
    idcu_http_response_set_header(response, "Content-Type", "application/json");
    idcu_http_response_set_json(response, "{\"message\": \"Hello, JSON!\"}");
}
```

### 注册路由

```c
idcu_http_server_get(&server, "/", hello_handler, NULL);
idcu_http_server_get(&server, "/api/json", json_handler, NULL);
```

### 启动服务器

```c
idcu_http_server_listen(&server, "0.0.0.0", 8080, 10);
```

### 停止服务器

```c
idcu_http_server_stop(&server);
idcu_http_server_destroy(&server);
```

### 静态文件服务

```c
idcu_http_server_set_static_dir(&server, "./public");
```

### 访问查询参数

```c
void user_handler(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data)
{
    const char* name = NULL;
    idcu_http_request_get_query_param(request, "name", &name);
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Hello, %s!", name ? name : "Guest");
    
    idcu_http_response_set_status(response, IDCU_HTTP_STATUS_OK);
    idcu_http_response_set_string(response, buffer);
}
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

## HTTP 状态码

常见状态码：
- 200 OK
- 201 Created
- 400 Bad Request
- 404 Not Found
- 500 Internal Server Error

## API 文档

详见 [include/idcu/http/http_server.h](include/idcu/http/http_server.h)
```

## 验证检查清单

- [ ] HTTP 服务器头文件已创建
- [ ] HTTP 服务器实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 服务器可以监听端口
- [ ] 可以注册和访问路由
- [ ] 可以返回 JSON 响应

## Git 提交

```bash
git add libs/idcu-http-server/
git commit -m "feat: add idcu-http-server library

- Add HTTP/1.1 protocol support
- Add route-based request handling
- Add GET/POST/PUT/DELETE/HEAD/OPTIONS/PATCH methods
- Add HTTP header parsing and setting
- Add query parameter parsing
- Add JSON request/response support
- Add static file serving
- Add form data parsing
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 端口被占用 | 端口已被其他程序使用 | 更换端口或关闭占用程序 |
| 路由不匹配 | 路径错误 | 确保路径完全匹配 |
| 响应错误 | 未设置 Content-Type | 确保设置正确的 Content-Type |
