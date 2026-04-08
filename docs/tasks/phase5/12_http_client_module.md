# 任务 5.12: http-client-module - HTTP客户端业务模块

## 目标

创建HTTP客户端业务模块，支持：
- HTTP/HTTPS请求
- 连接池管理
- 请求重试
- 请求超时
- 代理支持
- Cookie管理
- 认证支持
- 异步请求

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/business/http-client-module/include/idcu/http_client_module
mkdir -p modules/business/http-client-module/src/idcu/http_client_module
mkdir -p modules/business/http-client-module/tests
mkdir -p modules/business/http-client-module/examples
```

### 2. 创建HTTP客户端模块头文件 (http_client_module.h)

创建 `modules/business/http-client-module/include/idcu/http_client_module/http_client_module.h`：

```c
#ifndef IDCU_HTTP_CLIENT_MODULE_HTTP_CLIENT_MODULE_H
#define IDCU_HTTP_CLIENT_MODULE_HTTP_CLIENT_MODULE_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include "idcu/sdk/sdk.h"
#include "idcu/http_client/http_client.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_HttpRequestId;
typedef uint64_t idcu_HttpConnectionId;

typedef enum
{
    IDCU_HTTP_METHOD_GET = 0,
    IDCU_HTTP_METHOD_POST,
    IDCU_HTTP_METHOD_PUT,
    IDCU_HTTP_METHOD_DELETE,
    IDCU_HTTP_METHOD_PATCH,
    IDCU_HTTP_METHOD_HEAD,
    IDCU_HTTP_METHOD_OPTIONS
} idcu_HttpMethod;

typedef enum
{
    IDCU_HTTP_AUTH_NONE = 0,
    IDCU_HTTP_AUTH_BASIC,
    IDCU_HTTP_AUTH_BEARER,
    IDCU_HTTP_AUTH_DIGEST,
    IDCU_HTTP_AUTH_OAUTH2,
    IDCU_HTTP_AUTH_CUSTOM
} idcu_HttpAuthType;

typedef struct
{
    char name[256];
    char value[1024];
} idcu_HttpHeader;

typedef struct
{
    char name[256];
    char value[4096];
    uint64_t expires;
    char domain[256];
    char path[256];
    int secure;
    int http_only;
} idcu_HttpCookie;

typedef struct
{
    idcu_HttpRequestId request_id;
    char url[2048];
    idcu_HttpMethod method;
    idcu_Vector headers;
    idcu_Vector cookies;
    char* body;
    size_t body_size;
    idcu_HttpAuthType auth_type;
    char auth_username[256];
    char auth_password[256];
    char auth_token[1024];
    uint64_t timeout_ms;
    int follow_redirects;
    int max_redirects;
    int verify_ssl;
    char ca_cert_path[1024];
    char proxy_url[1024];
} idcu_HttpRequest;

typedef struct
{
    idcu_HttpRequestId request_id;
    int status_code;
    char status_text[256];
    idcu_Vector headers;
    idcu_Vector cookies;
    char* body;
    size_t body_size;
    uint64_t total_time_ms;
    uint64_t connect_time_ms;
    uint64_t start_transfer_time_ms;
    uint64_t size_download;
    uint64_t size_upload;
} idcu_HttpResponse;

typedef struct
{
    idcu_HttpConnectionId connection_id;
    char url[2048];
    int in_use;
    uint64_t last_used;
    void* handle;
} idcu_HttpConnection;

typedef struct
{
    char config_path[1024];
    uint64_t default_timeout_ms;
    int default_follow_redirects;
    int default_max_redirects;
    int default_verify_ssl;
    char default_ca_cert_path[1024];
    char default_proxy_url[1024];
    int enable_connection_pool;
    size_t max_connections;
    size_t max_connections_per_host;
    uint64_t connection_ttl_ms;
    int enable_cookie_jar;
    char cookie_jar_path[1024];
    int enable_request_logging;
    char request_log_path[1024];
} idcu_HttpClientModuleConfig;

typedef void (*idcu_HttpResponseCallback)(idcu_HttpResponse* response, void* user_data);
typedef void (*idcu_HttpErrorCallback)(int error_code, const char* error_message, void* user_data);

typedef struct
{
    idcu_SdkContext* sdk;
    idcu_HttpClientModuleConfig config;
    
    idcu_Vector connections;
    idcu_Mutex pool_lock;
    
    idcu_Vector cookies;
    idcu_Mutex cookie_lock;
    
    idcu_HttpClient* client;
    
    int initialized;
    int running;
} idcu_HttpClientModule;

int  idcu_http_client_module_config_init(idcu_HttpClientModuleConfig* config);

int  idcu_http_client_module_init(idcu_HttpClientModule* module, idcu_SdkContext* sdk,
                                    const idcu_HttpClientModuleConfig* config);
void idcu_http_client_module_destroy(idcu_HttpClientModule* module);

int  idcu_http_client_module_start(idcu_HttpClientModule* module);
int  idcu_http_client_module_stop(idcu_HttpClientModule* module);

int  idcu_http_request_init(idcu_HttpRequest* request, const char* url, idcu_HttpMethod method);
void idcu_http_request_destroy(idcu_HttpRequest* request);

int  idcu_http_request_add_header(idcu_HttpRequest* request, const char* name, const char* value);
int  idcu_http_request_add_cookie(idcu_HttpRequest* request, const idcu_HttpCookie* cookie);
int  idcu_http_request_set_body(idcu_HttpRequest* request, const void* body, size_t body_size);
int  idcu_http_request_set_auth(idcu_HttpRequest* request, idcu_HttpAuthType type,
                                  const char* username, const char* password, const char* token);
int  idcu_http_request_set_timeout(idcu_HttpRequest* request, uint64_t timeout_ms);
int  idcu_http_request_set_proxy(idcu_HttpRequest* request, const char* proxy_url);

int  idcu_http_client_module_execute(idcu_HttpClientModule* module, idcu_HttpRequest* request,
                                       idcu_HttpResponse* response);
int  idcu_http_client_module_execute_async(idcu_HttpClientModule* module, idcu_HttpRequest* request,
                                             idcu_HttpResponseCallback response_cb,
                                             idcu_HttpErrorCallback error_cb,
                                             void* user_data);

int  idcu_http_response_get_header(idcu_HttpResponse* response, const char* name, 
                                     char* value, size_t value_size);
int  idcu_http_response_get_cookie(idcu_HttpResponse* response, const char* name,
                                     idcu_HttpCookie* cookie);
int  idcu_http_response_is_success(idcu_HttpResponse* response);

int  idcu_http_client_module_get(idcu_HttpClientModule* module, const char* url,
                                   idcu_HttpResponse* response);
int  idcu_http_client_module_post(idcu_HttpClientModule* module, const char* url,
                                    const void* body, size_t body_size,
                                    idcu_HttpResponse* response);
int  idcu_http_client_module_put(idcu_HttpClientModule* module, const char* url,
                                   const void* body, size_t body_size,
                                   idcu_HttpResponse* response);
int  idcu_http_client_module_delete(idcu_HttpClientModule* module, const char* url,
                                      idcu_HttpResponse* response);

int  idcu_http_client_module_add_cookie(idcu_HttpClientModule* module, const idcu_HttpCookie* cookie);
int  idcu_http_client_module_remove_cookie(idcu_HttpClientModule* module, const char* name);
int  idcu_http_client_module_clear_cookies(idcu_HttpClientModule* module);
int  idcu_http_client_module_save_cookies(idcu_HttpClientModule* module, const char* path);
int  idcu_http_client_module_load_cookies(idcu_HttpClientModule* module, const char* path);

int  idcu_http_client_module_get_connection_count(idcu_HttpClientModule* module, size_t* count);
int  idcu_http_client_module_clear_connection_pool(idcu_HttpClientModule* module);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建模块配置文件 (module.yaml)

创建 `modules/business/http-client-module/module.yaml`：

```yaml
name: http-client-module
version: 1.0.0
description: HTTP client business module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-sdk
  - idcu-http-client
  - idcu-log

build:
  type: cmake
  targets:
    - http-client-module

headers:
  - idcu/http_client_module/http_client_module.h

features:
  - http: HTTP/HTTPS requests
  - connection_pool: Connection pool management
  - retry: Request retry
  - timeout: Request timeout
  - proxy: Proxy support
  - cookies: Cookie management
  - auth: Authentication support
  - async: Asynchronous requests

testing:
  enabled: true
  framework: internal
```

### 4. 创建 README.md

创建 `modules/business/http-client-module/README.md`：

```markdown
# http-client-module

IDCU Agent 的HTTP客户端业务模块。

## 功能特性

- **HTTP/HTTPS**: HTTP/HTTPS请求
- **连接池**: 连接池管理
- **请求重试**: 请求重试
- **请求超时**: 请求超时
- **代理支持**: 代理支持
- **Cookie管理**: Cookie管理
- **认证支持**: 认证支持
- **异步请求**: 异步请求

## 快速开始

### 初始化HTTP客户端模块

```c
#include "idcu/http_client_module/http_client_module.h"

idcu_HttpClientModuleConfig config;
idcu_http_client_module_config_init(&config);

config.default_timeout_ms = 30000;
config.default_follow_redirects = 1;
config.default_max_redirects = 5;
config.default_verify_ssl = 1;
config.enable_connection_pool = 1;
config.max_connections = 100;
config.max_connections_per_host = 10;
config.enable_cookie_jar = 1;

idcu_HttpClientModule module;
idcu_http_client_module_init(&module, sdk_context, &config);
```

### 启动HTTP客户端模块

```c
idcu_http_client_module_start(&module);
```

### 简单GET请求

```c
idcu_HttpResponse response;
idcu_http_client_module_get(&module, "https://api.example.com/data", &response);

if (idcu_http_response_is_success(&response)) {
    printf("Response: %.*s\n", (int)response.body_size, response.body);
}

idcu_http_response_destroy(&response);
```

### 构建复杂请求

```c
idcu_HttpRequest request;
idcu_http_request_init(&request, "https://api.example.com/submit", IDCU_HTTP_METHOD_POST);

idcu_http_request_add_header(&request, "Content-Type", "application/json");
idcu_http_request_add_header(&request, "User-Agent", "IDCU-Agent/1.0");

const char* json_body = "{\"data\":\"test\"}";
idcu_http_request_set_body(&request, json_body, strlen(json_body));

idcu_http_request_set_auth(&request, IDCU_HTTP_AUTH_BASIC, 
                             "username", "password", NULL);
idcu_http_request_set_timeout(&request, 60000);

idcu_HttpResponse response;
idcu_http_client_module_execute(&module, &request, &response);

idcu_http_request_destroy(&request);
idcu_http_response_destroy(&response);
```

### POST请求

```c
const char* form_data = "field1=value1&field2=value2";
idcu_HttpResponse response;
idcu_http_client_module_post(&module, "https://api.example.com/submit",
                               form_data, strlen(form_data), &response);
idcu_http_response_destroy(&response);
```

### PUT请求

```c
const char* data = "updated data";
idcu_HttpResponse response;
idcu_http_client_module_put(&module, "https://api.example.com/resource/1",
                              data, strlen(data), &response);
idcu_http_response_destroy(&response);
```

### DELETE请求

```c
idcu_HttpResponse response;
idcu_http_client_module_delete(&module, "https://api.example.com/resource/1", &response);
idcu_http_response_destroy(&response);
```

### 异步请求

```c
void on_response(idcu_HttpResponse* response, void* user_data)
{
    if (idcu_http_response_is_success(response)) {
        printf("Async response: %.*s\n", (int)response->body_size, response->body);
    }
}

void on_error(int error_code, const char* error_message, void* user_data)
{
    printf("Error: %s\n", error_message);
}

idcu_HttpRequest request;
idcu_http_request_init(&request, "https://api.example.com/data", IDCU_HTTP_METHOD_GET);

idcu_http_client_module_execute_async(&module, &request, 
                                         on_response, on_error, NULL);

idcu_http_request_destroy(&request);
```

### Cookie管理

```c
idcu_HttpCookie cookie = {
    .name = "session_id",
    .value = "abc123",
    .domain = "example.com",
    .path = "/",
    .secure = 1,
    .http_only = 1
};

idcu_http_client_module_add_cookie(&module, &cookie);

idcu_http_client_module_save_cookies(&module, "./cookies.json");
```

### 获取响应头

```c
char content_type[256];
idcu_http_response_get_header(&response, "Content-Type", 
                                content_type, sizeof(content_type));
```

### 停止HTTP客户端模块

```c
idcu_http_client_module_stop(&module);
idcu_http_client_module_destroy(&module);
```

## HTTP方法

| 方法 | 说明 |
|-----|------|
| GET | GET请求 |
| POST | POST请求 |
| PUT | PUT请求 |
| DELETE | DELETE请求 |
| PATCH | PATCH请求 |
| HEAD | HEAD请求 |
| OPTIONS | OPTIONS请求 |

## 认证类型

| 类型 | 说明 |
|-----|------|
| NONE | 无认证 |
| BASIC | Basic认证 |
| BEARER | Bearer认证 |
| DIGEST | Digest认证 |
| OAUTH2 | OAuth2认证 |
| CUSTOM | 自定义认证 |

## API 文档

详见 [include/idcu/http_client_module/http_client_module.h](include/idcu/http_client_module/http_client_module.h)
```

## 验证检查清单

- [ ] HTTP客户端模块头文件已创建
- [ ] HTTP客户端模块实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以发送HTTP请求
- [ ] 连接池功能正常工作
- [ ] Cookie管理功能正常

## Git 提交

```bash
git add modules/business/http-client-module/
git commit -m "feat: add http-client-module business module

- Add HTTP/HTTPS requests
- Add connection pool management
- Add request retry
- Add request timeout
- Add proxy support
- Add cookie management
- Add authentication support
- Add asynchronous requests
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 请求超时 | 网络问题或超时设置太短 | 检查网络连接，增加超时时间 |
| SSL证书错误 | SSL证书验证失败 | 检查CA证书路径或禁用SSL验证 |
| 连接池耗尽 | 连接数限制太小 | 增加最大连接数 |
