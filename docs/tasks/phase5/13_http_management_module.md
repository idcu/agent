# 任务 5.13: http-management-module - HTTP管理业务模块

## 目标

创建HTTP管理业务模块，支持：
- RESTful API管理接口
- 模块生命周期管理
- 配置热重载
- 健康检查接口
- 指标暴露
- 日志管理
- Web UI集成

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/business/http-management-module/include/idcu/http_management_module
mkdir -p modules/business/http-management-module/src/idcu/http_management_module
mkdir -p modules/business/http-management-module/tests
mkdir -p modules/business/http-management-module/examples
```

### 2. 创建HTTP管理模块头文件 (http_management_module.h)

创建 `modules/business/http-management-module/include/idcu/http_management_module/http_management_module.h`：

```c
#ifndef IDCU_HTTP_MANAGEMENT_MODULE_HTTP_MANAGEMENT_MODULE_H
#define IDCU_HTTP_MANAGEMENT_MODULE_HTTP_MANAGEMENT_MODULE_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include "idcu/sdk/sdk.h"
#include "idcu/http_server/http_server.h"
#include "idcu/metrics/metrics.h"
#include "idcu/healthcheck/healthcheck.h"
#include "idcu/log/log.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_MgmtRouteId;
typedef uint64_t idcu_MgmtSessionId;

typedef enum
{
    IDCU_MGMT_HANDLER_GET = 0,
    IDCU_MGMT_HANDLER_POST,
    IDCU_MGMT_HANDLER_PUT,
    IDCU_MGMT_HANDLER_DELETE,
    IDCU_MGMT_HANDLER_PATCH
} idcu_MgmtHandlerType;

typedef struct
{
    idcu_MgmtRouteId route_id;
    char path[512];
    idcu_MgmtHandlerType method;
    char description[512];
    int require_auth;
    void* handler_data;
} idcu_MgmtRoute;

typedef struct
{
    int status_code;
    char content_type[256];
    char* body;
    size_t body_size;
    idcu_Vector headers;
} idcu_MgmtResponse;

typedef struct
{
    idcu_MgmtSessionId session_id;
    char username[128];
    char token[256];
    uint64_t created_time;
    uint64_t last_access;
    uint64_t expires;
    char ip_address[64];
    char user_agent[512];
    int is_admin;
} idcu_MgmtSession;

typedef struct
{
    char bind_address[256];
    uint16_t bind_port;
    char config_path[1024];
    int enable_auth;
    char admin_username[128];
    char admin_password_hash[256];
    char admin_salt[128];
    uint64_t session_timeout_ms;
    int enable_ui;
    char ui_path[1024];
    int enable_cors;
    char cors_origin[1024];
    int enable_https;
    char cert_path[1024];
    char key_path[1024];
    int enable_metrics_endpoint;
    char metrics_path[256];
    int enable_health_endpoint;
    char health_path[256];
} idcu_HttpManagementModuleConfig;

typedef int (*idcu_MgmtRequestHandler)(idcu_HttpRequest* request, 
                                           idcu_MgmtResponse* response,
                                           void* user_data);

typedef struct
{
    idcu_SdkContext* sdk;
    idcu_HttpManagementModuleConfig config;
    
    idcu_HttpServer* server;
    idcu_Vector routes;
    idcu_Mutex routes_lock;
    
    idcu_HashMap sessions;
    idcu_Mutex sessions_lock;
    
    idcu_MetricsRegistry* metrics;
    idcu_HealthCheck* health_check;
    
    int initialized;
    int running;
} idcu_HttpManagementModule;

int  idcu_http_management_module_config_init(idcu_HttpManagementModuleConfig* config);

int  idcu_http_management_module_init(idcu_HttpManagementModule* module, idcu_SdkContext* sdk,
                                         const idcu_HttpManagementModuleConfig* config);
void idcu_http_management_module_destroy(idcu_HttpManagementModule* module);

int  idcu_http_management_module_start(idcu_HttpManagementModule* module);
int  idcu_http_management_module_stop(idcu_HttpManagementModule* module);

int  idcu_http_management_module_add_route(idcu_HttpManagementModule* module,
                                            const char* path,
                                            idcu_MgmtHandlerType method,
                                            idcu_MgmtRequestHandler handler,
                                            const char* description,
                                            int require_auth,
                                            void* user_data,
                                            idcu_MgmtRouteId* out_route_id);
int  idcu_http_management_module_remove_route(idcu_HttpManagementModule* module,
                                               idcu_MgmtRouteId route_id);

int  idcu_mgmt_response_init(idcu_MgmtResponse* response);
void idcu_mgmt_response_destroy(idcu_MgmtResponse* response);

int  idcu_mgmt_response_set_status(idcu_MgmtResponse* response, int status_code);
int  idcu_mgmt_response_set_header(idcu_MgmtResponse* response, const char* name, const char* value);
int  idcu_mgmt_response_set_body(idcu_MgmtResponse* response, const void* body, size_t body_size);
int  idcu_mgmt_response_set_json(idcu_MgmtResponse* response, const char* json);
int  idcu_mgmt_response_set_text(idcu_MgmtResponse* response, const char* text);
int  idcu_mgmt_response_set_html(idcu_MgmtResponse* response, const char* html);

int  idcu_mgmt_response_ok(idcu_MgmtResponse* response, const char* message);
int  idcu_mgmt_response_error(idcu_MgmtResponse* response, int status_code, const char* message);
int  idcu_mgmt_response_not_found(idcu_MgmtResponse* response, const char* message);
int  idcu_mgmt_response_forbidden(idcu_MgmtResponse* response, const char* message);
int  idcu_mgmt_response_unauthorized(idcu_MgmtResponse* response, const char* message);

int  idcu_http_management_module_create_session(idcu_HttpManagementModule* module,
                                                 const char* username,
                                                 const char* password,
                                                 idcu_MgmtSession* out_session);
int  idcu_http_management_module_validate_session(idcu_HttpManagementModule* module,
                                                    const char* token,
                                                    idcu_MgmtSession* out_session);
int  idcu_http_management_module_destroy_session(idcu_HttpManagementModule* module,
                                                   idcu_MgmtSessionId session_id);
int  idcu_http_management_module_cleanup_sessions(idcu_HttpManagementModule* module);

int  idcu_http_management_module_reload_config(idcu_HttpManagementModule* module);
int  idcu_http_management_module_get_status(idcu_HttpManagementModule* module, 
                                              char* buffer, size_t buffer_size);
int  idcu_http_management_module_get_modules(idcu_HttpManagementModule* module,
                                               char* buffer, size_t buffer_size);
int  idcu_http_management_module_get_config(idcu_HttpManagementModule* module,
                                              char* buffer, size_t buffer_size);
int  idcu_http_management_module_set_config(idcu_HttpManagementModule* module,
                                              const char* config_json);

int  idcu_http_management_module_restart_module(idcu_HttpManagementModule* module,
                                                   const char* module_name);
int  idcu_http_management_module_stop_module(idcu_HttpManagementModule* module,
                                                const char* module_name);

int  idcu_http_management_module_get_logs(idcu_HttpManagementModule* module,
                                            uint64_t lines,
                                            char* buffer, size_t buffer_size);
int  idcu_http_management_module_set_log_level(idcu_HttpManagementModule* module,
                                                   idcu_LogLevel level);

int  idcu_http_management_module_get_health(idcu_HttpManagementModule* module,
                                              char* buffer, size_t buffer_size);
int  idcu_http_management_module_get_metrics(idcu_HttpManagementModule* module,
                                               char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建模块配置文件 (module.yaml)

创建 `modules/business/http-management-module/module.yaml`：

```yaml
name: http-management-module
version: 1.0.0
description: HTTP management business module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-sdk
  - idcu-http-server
  - idcu-metrics
  - idcu-healthcheck
  - idcu-log
  - idcu-json
  - idcu-yaml

build:
  type: cmake
  targets:
    - http-management-module

headers:
  - idcu/http_management_module/http_management_module.h

features:
  - rest_api: RESTful API management interface
  - lifecycle: Module lifecycle management
  - config_reload: Config hot reload
  - health_check: Health check endpoint
  - metrics: Metrics exposure
  - logging: Log management
  - web_ui: Web UI integration

testing:
  enabled: true
  framework: internal
```

### 4. 创建 README.md

创建 `modules/business/http-management-module/README.md`：

```markdown
# http-management-module

IDCU Agent 的HTTP管理业务模块。

## 功能特性

- **RESTful API**: RESTful API管理接口
- **生命周期管理**: 模块生命周期管理
- **配置热重载**: 配置热重载
- **健康检查接口**: 健康检查接口
- **指标暴露**: 指标暴露
- **日志管理**: 日志管理
- **Web UI集成**: Web UI集成

## 快速开始

### 初始化HTTP管理模块

```c
#include "idcu/http_management_module/http_management_module.h"

idcu_HttpManagementModuleConfig config;
idcu_http_management_module_config_init(&config);

config.bind_address = "0.0.0.0";
config.bind_port = 8080;
config.enable_auth = 1;
strncpy(config.admin_username, "admin", sizeof(config.admin_username));
config.session_timeout_ms = 3600000;
config.enable_ui = 1;
config.enable_metrics_endpoint = 1;
config.enable_health_endpoint = 1;

idcu_HttpManagementModule module;
idcu_http_management_module_init(&module, sdk_context, &config);
```

### 启动HTTP管理模块

```c
idcu_http_management_module_start(&module);
```

### 添加自定义路由

```c
int custom_handler(idcu_HttpRequest* request, idcu_MgmtResponse* response, void* user_data)
{
    idcu_mgmt_response_ok(response, "Custom handler called");
    return IDCU_ERR_OK;
}

idcu_MgmtRouteId route_id;
idcu_http_management_module_add_route(&module, "/api/custom", IDCU_MGMT_HANDLER_GET,
                                        custom_handler, "Custom endpoint", 0, NULL, &route_id);
```

### 创建响应

```c
idcu_MgmtResponse response;
idcu_mgmt_response_init(&response);

// JSON响应
idcu_mgmt_response_set_json(&response, "{\"status\":\"ok\"}");

// 文本响应
idcu_mgmt_response_set_text(&response, "Hello World");

// 成功响应
idcu_mgmt_response_ok(&response, "Operation successful");

// 错误响应
idcu_mgmt_response_error(&response, 400, "Bad request");

idcu_mgmt_response_destroy(&response);
```

### API端点示例

#### 健康检查
```
GET /health
```

#### 指标
```
GET /metrics
```

#### 系统状态
```
GET /api/status
```

#### 模块列表
```
GET /api/modules
```

#### 配置
```
GET /api/config
PUT /api/config
```

#### 日志
```
GET /api/logs?lines=100
POST /api/log/level
```

#### 模块管理
```
POST /api/modules/{name}/restart
POST /api/modules/{name}/stop
```

#### 配置重载
```
POST /api/config/reload
```

### 认证

使用Basic Auth认证：
```
Authorization: Basic YWRtaW46cGFzc3dvcmQ=
```

或者使用Session Token：
```
Authorization: Bearer <session_token>
```

### CORS支持

如果启用了CORS，模块会自动处理跨域请求。

### HTTPS支持

可以启用HTTPS来加密管理接口：
```c
config.enable_https = 1;
strncpy(config.cert_path, "./cert.pem", sizeof(config.cert_path));
strncpy(config.key_path, "./key.pem", sizeof(config.key_path));
```

### Web UI

如果启用了Web UI，可以通过浏览器访问管理界面：
```
http://localhost:8080/
```

### 停止HTTP管理模块

```c
idcu_http_management_module_stop(&module);
idcu_http_management_module_destroy(&module);
```

## HTTP方法处理类型

| 类型 | 说明 |
|-----|------|
| GET | GET请求 |
| POST | POST请求 |
| PUT | PUT请求 |
| DELETE | DELETE请求 |
| PATCH | PATCH请求 |

## API文档

详见 [include/idcu/http_management_module/http_management_module.h](include/idcu/http_management_module/http_management_module.h)
```

## 验证检查清单

- [ ] HTTP管理模块头文件已创建
- [ ] HTTP管理模块实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以启动HTTP管理服务
- [ ] API端点正常工作
- [ ] 认证功能正常工作

## Git提交

```bash
git add modules/business/http-management-module/
git commit -m "feat: add http-management-module business module

- Add RESTful API management interface
- Add module lifecycle management
- Add config hot reload
- Add health check endpoint
- Add metrics exposure
- Add log management
- Add web UI integration
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 端口被占用 | 端口已被其他程序使用 | 更换端口或关闭占用端口的程序 |
| 认证失败 | 用户名或密码错误 | 检查用户名和密码 |
| 无法访问UI | UI路径配置错误 | 检查UI路径配置 |
