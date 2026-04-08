# 任务 4.1: REST API 模块

## 目标

创建 REST API 模块，支持：
- HTTP API 端点
- 请求路由
- 参数验证
- 认证和授权
- API 版本控制
- 请求/响应日志
- 限流

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/rest-api/include/idcu/rest_api
mkdir -p modules/rest-api/src/idcu/rest_api
mkdir -p modules/rest-api/tests
mkdir -p modules/rest-api/examples
```

### 2. 创建 REST API 头文件 (rest_api.h)

创建 `modules/rest-api/include/idcu/rest_api/rest_api.h`：

```c
#ifndef IDCU_REST_API_REST_API_H
#define IDCU_REST_API_REST_API_H

#include "idcu/common/error_code.h"
#include "idcu/http/http_server.h"
#include "idcu/json/json.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_ApiEndpointId;

typedef enum
{
    IDCU_API_AUTH_NONE = 0,
    IDCU_API_AUTH_BASIC,
    IDCU_API_AUTH_BEARER,
    IDCU_API_AUTH_API_KEY,
    IDCU_API_AUTH_CUSTOM
} idcu_ApiAuthType;

typedef struct
{
    idcu_ApiAuthType type;
    char realm[128];
    char secret[256];
    char header[128];
    char query_param[128];
} idcu_ApiAuthConfig;

typedef struct
{
    int enabled;
    uint64_t requests_per_minute;
    uint64_t requests_per_hour;
    uint64_t requests_per_day;
} idcu_ApiRateLimitConfig;

typedef struct
{
    int enabled;
    char prefix[64];
    int version;
} idcu_ApiVersionConfig;

typedef int (*idcu_ApiHandler)(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data);

typedef struct
{
    idcu_ApiEndpointId id;
    char path[512];
    idcu_HttpMethod method;
    idcu_ApiHandler handler;
    void* user_data;
    idcu_ApiAuthConfig auth;
    idcu_ApiRateLimitConfig rate_limit;
    idcu_ApiVersionConfig version;
    char summary[512];
    char description[1024];
    int enabled;
} idcu_ApiEndpoint;

typedef struct
{
    idcu_HttpServer http_server;
    idcu_Vector endpoints;
    idcu_HashMap endpoints_by_path;
    idcu_Mutex lock;
    
    idcu_ApiAuthConfig default_auth;
    idcu_ApiRateLimitConfig default_rate_limit;
    idcu_ApiVersionConfig default_version;
    
    int log_requests;
    int log_responses;
    int cors_enabled;
    char cors_origins[1024];
    char cors_methods[256];
    char cors_headers[512];
    
    int running;
    int initialized;
} idcu_RestApi;

typedef struct
{
    char host[256];
    uint16_t port;
    int backlog;
    
    idcu_ApiAuthConfig default_auth;
    idcu_ApiRateLimitConfig default_rate_limit;
    idcu_ApiVersionConfig default_version;
    
    int log_requests;
    int log_responses;
    int cors_enabled;
    char cors_origins[1024];
    char cors_methods[256];
    char cors_headers[512];
} idcu_RestApiConfig;

int  idcu_rest_api_config_init(idcu_RestApiConfig* config);

int  idcu_rest_api_init(idcu_RestApi* api, const idcu_RestApiConfig* config);
void idcu_rest_api_destroy(idcu_RestApi* api);
int  idcu_rest_api_start(idcu_RestApi* api);
void idcu_rest_api_stop(idcu_RestApi* api);

idcu_ApiEndpointId idcu_rest_api_add_endpoint(idcu_RestApi* api, const char* path, idcu_HttpMethod method,
                                                idcu_ApiHandler handler, void* user_data);
idcu_ApiEndpointId idcu_rest_api_get(idcu_RestApi* api, const char* path, idcu_ApiHandler handler, void* user_data);
idcu_ApiEndpointId idcu_rest_api_post(idcu_RestApi* api, const char* path, idcu_ApiHandler handler, void* user_data);
idcu_ApiEndpointId idcu_rest_api_put(idcu_RestApi* api, const char* path, idcu_ApiHandler handler, void* user_data);
idcu_ApiEndpointId idcu_rest_api_delete(idcu_RestApi* api, const char* path, idcu_ApiHandler handler, void* user_data);
int  idcu_rest_api_remove_endpoint(idcu_RestApi* api, idcu_ApiEndpointId id);
int  idcu_rest_api_enable_endpoint(idcu_RestApi* api, idcu_ApiEndpointId id);
int  idcu_rest_api_disable_endpoint(idcu_RestApi* api, idcu_ApiEndpointId id);

idcu_ApiEndpoint* idcu_rest_api_get_endpoint(idcu_RestApi* api, idcu_ApiEndpointId id);
idcu_ApiEndpoint* idcu_rest_api_find_endpoint(idcu_RestApi* api, const char* path, idcu_HttpMethod method);

int  idcu_rest_api_set_auth(idcu_RestApi* api, idcu_ApiEndpointId id, const idcu_ApiAuthConfig* auth);
int  idcu_rest_api_set_rate_limit(idcu_RestApi* api, idcu_ApiEndpointId id, const idcu_ApiRateLimitConfig* rate_limit);
int  idcu_rest_api_set_version(idcu_RestApi* api, idcu_ApiEndpointId id, const idcu_ApiVersionConfig* version);
int  idcu_rest_api_set_summary(idcu_RestApi* api, idcu_ApiEndpointId id, const char* summary);
int  idcu_rest_api_set_description(idcu_RestApi* api, idcu_ApiEndpointId id, const char* description);

int  idcu_rest_api_enable_cors(idcu_RestApi* api, const char* origins, const char* methods, const char* headers);
int  idcu_rest_api_disable_cors(idcu_RestApi* api);

int  idcu_rest_api_enable_logging(idcu_RestApi* api, int log_requests, int log_responses);
int  idcu_rest_api_disable_logging(idcu_RestApi* api);

int  idcu_api_response_json(idcu_HttpResponse* response, const char* json);
int  idcu_api_response_json_value(idcu_HttpResponse* response, const idcu_JsonValue* json);
int  idcu_api_response_ok(idcu_HttpResponse* response, const char* message);
int  idcu_api_response_ok_json(idcu_HttpResponse* response, const char* json);
int  idcu_api_response_created(idcu_HttpResponse* response, const char* location, const char* json);
int  idcu_api_response_error(idcu_HttpResponse* response, idcu_HttpStatus status, const char* message);
int  idcu_api_response_bad_request(idcu_HttpResponse* response, const char* message);
int  idcu_api_response_unauthorized(idcu_HttpResponse* response, const char* message);
int  idcu_api_response_forbidden(idcu_HttpResponse* response, const char* message);
int  idcu_api_response_not_found(idcu_HttpResponse* response, const char* message);
int  idcu_api_response_internal_error(idcu_HttpResponse* response, const char* message);

int  idcu_api_get_query_param(const idcu_HttpRequest* request, const char* name, char* buffer, size_t buffer_size);
int  idcu_api_get_query_param_int(const idcu_HttpRequest* request, const char* name, int64_t* value, int64_t default_value);
int  idcu_api_get_query_param_double(const idcu_HttpRequest* request, const char* name, double* value, double default_value);
int  idcu_api_get_query_param_bool(const idcu_HttpRequest* request, const char* name, int* value, int default_value);

int  idcu_api_parse_json_body(const idcu_HttpRequest* request, idcu_JsonValue* result);
int  idcu_api_parse_json_body_alloc(const idcu_HttpRequest* request, idcu_JsonValue** result);

int  idcu_api_auth_basic_verify(const idcu_HttpRequest* request, const char* username, const char* password);
int  idcu_api_auth_bearer_verify(const idcu_HttpRequest* request, const char* token);
int  idcu_api_auth_api_key_verify(const idcu_HttpRequest* request, const char* header, const char* query_param, const char* key);

int  idcu_api_generate_openapi(const idcu_RestApi* api, char* buffer, size_t buffer_size);
int  idcu_api_generate_openapi_json(const idcu_RestApi* api, idcu_JsonValue* result);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `modules/rest-api/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(rest-api VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(rest-api STATIC
    src/idcu/rest_api/rest_api.c
)

target_include_directories(rest-api PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(rest-api PRIVATE
    idcu::common
    idcu::http-server
    idcu::json
    idcu::log
    idcu::config
)

add_library(idcu::rest-api ALIAS rest-api)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `modules/rest-api/module.yaml`：

```yaml
name: rest-api
version: 1.0.0
description: REST API module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-http-server
  - idcu-json
  - idcu-log
  - idcu-config

build:
  type: cmake
  targets:
    - rest-api

headers:
  - idcu/rest_api/rest_api.h

features:
  - endpoints: HTTP API endpoints
  - routing: Request routing
  - validation: Parameter validation
  - auth: Authentication and authorization
  - versioning: API versioning
  - logging: Request/response logging
  - rate_limit: Rate limiting
  - cors: CORS support
  - openapi: OpenAPI documentation generation

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `modules/rest-api/README.md`：

```markdown
# rest-api

IDCU Agent 的 REST API 模块。

## 功能特性

- **API 端点**: HTTP API 端点
- **请求路由**: 请求路由
- **参数验证**: 参数验证
- **认证授权**: 认证和授权
- **版本控制**: API 版本控制
- **请求日志**: 请求/响应日志
- **限流**: 限流
- **CORS**: CORS 支持
- **OpenAPI**: OpenAPI 文档生成

## 快速开始

### 初始化 REST API

```c
#include "idcu/rest_api/rest_api.h"

idcu_RestApiConfig config;
idcu_rest_api_config_init(&config);

strncpy(config.host, "0.0.0.0", sizeof(config.host));
config.port = 8080;
config.backlog = 10;
config.log_requests = 1;
config.log_responses = 1;
config.cors_enabled = 1;
strncpy(config.cors_origins, "*", sizeof(config.cors_origins));

idcu_RestApi api;
idcu_rest_api_init(&api, &config);
```

### 添加 API 端点

```c
int hello_handler(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data)
{
    return idcu_api_response_ok(response, "Hello, World!");
}

idcu_rest_api_get(&api, "/api/hello", hello_handler, NULL);
```

### JSON 响应

```c
int user_handler(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data)
{
    const char* json = "{"
        "\"id\": 1,"
        "\"name\": \"John Doe\","
        "\"email\": \"john@example.com\""
    "}";
    
    return idcu_api_response_ok_json(response, json);
}

idcu_rest_api_get(&api, "/api/users/1", user_handler, NULL);
```

### 创建资源

```c
int create_user_handler(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data)
{
    idcu_JsonValue body;
    if (idcu_api_parse_json_body(request, &body) != IDCU_ERR_OK) {
        return idcu_api_response_bad_request(response, "Invalid JSON");
    }
    
    const char* json = "{"
        "\"id\": 2,"
        "\"name\": \"Jane Doe\","
        "\"email\": \"jane@example.com\""
    "}";
    
    return idcu_api_response_created(response, "/api/users/2", json);
}

idcu_rest_api_post(&api, "/api/users", create_user_handler, NULL);
```

### 错误响应

```c
int not_found_handler(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data)
{
    return idcu_api_response_not_found(response, "Resource not found");
}
```

### 查询参数

```c
int search_handler(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data)
{
    char query[256];
    if (idcu_api_get_query_param(request, "q", query, sizeof(query)) == IDCU_ERR_OK) {
        char result[512];
        snprintf(result, sizeof(result), "{\"query\": \"%s\"}", query);
        return idcu_api_response_ok_json(response, result);
    }
    
    return idcu_api_response_bad_request(response, "Missing 'q' parameter");
}

idcu_rest_api_get(&api, "/api/search", search_handler, NULL);
```

### Basic Auth

```c
idcu_ApiAuthConfig auth;
auth.type = IDCU_API_AUTH_BASIC;
strncpy(auth.realm, "API", sizeof(auth.realm));

idcu_ApiEndpointId id = idcu_rest_api_get(&api, "/api/protected", protected_handler, NULL);
idcu_rest_api_set_auth(&api, id, &auth);
```

### Bearer Token

```c
idcu_ApiAuthConfig auth;
auth.type = IDCU_API_AUTH_BEARER;

idcu_ApiEndpointId id = idcu_rest_api_get(&api, "/api/protected", protected_handler, NULL);
idcu_rest_api_set_auth(&api, id, &auth);
```

### API Key

```c
idcu_ApiAuthConfig auth;
auth.type = IDCU_API_AUTH_API_KEY;
strncpy(auth.header, "X-API-Key", sizeof(auth.header));
strncpy(auth.secret, "my-secret-key", sizeof(auth.secret));

idcu_ApiEndpointId id = idcu_rest_api_get(&api, "/api/protected", protected_handler, NULL);
idcu_rest_api_set_auth(&api, id, &auth);
```

### 限流

```c
idcu_ApiRateLimitConfig rate_limit;
rate_limit.enabled = 1;
rate_limit.requests_per_minute = 100;
rate_limit.requests_per_hour = 1000;

idcu_ApiEndpointId id = idcu_rest_api_get(&api, "/api/limited", limited_handler, NULL);
idcu_rest_api_set_rate_limit(&api, id, &rate_limit);
```

### API 版本

```c
idcu_ApiVersionConfig version;
version.enabled = 1;
strncpy(version.prefix, "/v1", sizeof(version.prefix));
version.version = 1;

idcu_ApiEndpointId id = idcu_rest_api_get(&api, "/api/users", users_handler, NULL);
idcu_rest_api_set_version(&api, id, &version);
```

### OpenAPI 文档

```c
char openapi[8192];
idcu_api_generate_openapi(&api, openapi, sizeof(openapi));
printf("%s\n", openapi);
```

### 启动 API

```c
idcu_rest_api_start(&api);
```

### 停止 API

```c
idcu_rest_api_stop(&api);
idcu_rest_api_destroy(&api);
```

## API 文档

详见 [include/idcu/rest_api/rest_api.h](include/idcu/rest_api/rest_api.h)
```

## 验证检查清单

- [ ] REST API 头文件已创建
- [ ] REST API 实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以添加和访问 API 端点
- [ ] JSON 响应正常工作
- [ ] 认证可以正常工作

## Git 提交

```bash
git add modules/rest-api/
git commit -m "feat: add rest-api module

- Add HTTP API endpoints
- Add request routing
- Add parameter validation
- Add authentication and authorization
- Add API versioning
- Add request/response logging
- Add rate limiting
- Add CORS support
- Add OpenAPI documentation generation
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 路由不匹配 | 路径错误 | 确保路径完全匹配 |
| 认证失败 | 凭据错误 | 检查认证凭据 |
| 限流触发 | 请求太多 | 减少请求频率或增加限流阈值 |
