# 任务 4.5: network-integration - 网络集成模块

## 目标

创建网络集成模块，支持：
- 统一的网络接口
- 连接池集成
- HTTP 客户端集成
- HTTP 服务器集成
- 消息总线集成
- 节点发现集成
- 网络监控
- 网络健康检查
- 网络配置管理

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/network-integration/include/idcu/network_integration
mkdir -p modules/network-integration/src/idcu/network_integration
mkdir -p modules/network-integration/tests
mkdir -p modules/network-integration/examples
```

### 2. 创建网络集成头文件 (network_integration.h)

创建 `modules/network-integration/include/idcu/network_integration/network_integration.h`：

```c
#ifndef IDCU_NETWORK_INTEGRATION_NETWORK_INTEGRATION_H
#define IDCU_NETWORK_INTEGRATION_NETWORK_INTEGRATION_H

#include "idcu/common/error_code.h"
#include "idcu/network/network.h"
#include "idcu/conn_pool/conn_pool.h"
#include "idcu/http/client.h"
#include "idcu/http/server.h"
#include "idcu/msgbus/msg_bus.h"
#include "idcu/discovery/discovery.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_NetworkIntegrationId;

typedef enum
{
    IDCU_NETWORK_STATUS_DISCONNECTED = 0,
    IDCU_NETWORK_STATUS_CONNECTING,
    IDCU_NETWORK_STATUS_CONNECTED,
    IDCU_NETWORK_STATUS_ERROR
} idcu_NetworkStatus;

typedef struct
{
    char name[128];
    char host[256];
    uint16_t port;
    int enabled;
    int auto_reconnect;
    uint64_t reconnect_delay_ms;
    uint64_t connect_timeout_ms;
    uint64_t request_timeout_ms;
} idcu_NetworkEndpoint;

typedef struct
{
    idcu_Vector endpoints;
    idcu_HashMap endpoints_by_name;
    idcu_Mutex lock;
    
    idcu_ConnPool* conn_pool;
    idcu_HttpClient* http_client;
    idcu_HttpServer* http_server;
    idcu_MsgBus* msg_bus;
    idcu_Discovery* discovery;
    
    idcu_NetworkStatus status;
    uint64_t last_check_at;
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint64_t request_count;
    uint64_t error_count;
    
    int initialized;
} idcu_NetworkIntegration;

typedef struct
{
    idcu_ConnPoolConfig conn_pool_config;
    idcu_HttpClientConfig http_client_config;
    idcu_HttpServerConfig http_server_config;
    idcu_MsgBusConfig msg_bus_config;
    idcu_DiscoveryConfig discovery_config;
    
    int enable_conn_pool;
    int enable_http_client;
    int enable_http_server;
    int enable_msg_bus;
    int enable_discovery;
    
    int enable_monitoring;
    uint64_t health_check_interval_ms;
} idcu_NetworkIntegrationConfig;

int  idcu_network_integration_config_init(idcu_NetworkIntegrationConfig* config);

int  idcu_network_integration_init(idcu_NetworkIntegration* ni, const idcu_NetworkIntegrationConfig* config);
void idcu_network_integration_destroy(idcu_NetworkIntegration* ni);
int  idcu_network_integration_start(idcu_NetworkIntegration* ni);
void idcu_network_integration_stop(idcu_NetworkIntegration* ni);

idcu_NetworkIntegrationId idcu_network_integration_add_endpoint(idcu_NetworkIntegration* ni, const idcu_NetworkEndpoint* endpoint);
int  idcu_network_integration_remove_endpoint(idcu_NetworkIntegration* ni, idcu_NetworkIntegrationId id);
idcu_NetworkEndpoint* idcu_network_integration_get_endpoint(idcu_NetworkIntegration* ni, idcu_NetworkIntegrationId id);
idcu_NetworkEndpoint* idcu_network_integration_get_endpoint_by_name(idcu_NetworkIntegration* ni, const char* name);
int  idcu_network_integration_enable_endpoint(idcu_NetworkIntegration* ni, idcu_NetworkIntegrationId id);
int  idcu_network_integration_disable_endpoint(idcu_NetworkIntegration* ni, idcu_NetworkIntegrationId id);

idcu_ConnPool* idcu_network_integration_get_conn_pool(idcu_NetworkIntegration* ni);
idcu_HttpClient* idcu_network_integration_get_http_client(idcu_NetworkIntegration* ni);
idcu_HttpServer* idcu_network_integration_get_http_server(idcu_NetworkIntegration* ni);
idcu_MsgBus* idcu_network_integration_get_msg_bus(idcu_NetworkIntegration* ni);
idcu_Discovery* idcu_network_integration_get_discovery(idcu_NetworkIntegration* ni);

int  idcu_network_integration_http_get(idcu_NetworkIntegration* ni, const char* url, idcu_HttpResponse* response);
int  idcu_network_integration_http_post(idcu_NetworkIntegration* ni, const char* url, const char* body, 
                                        size_t body_len, idcu_HttpResponse* response);
int  idcu_network_integration_http_put(idcu_NetworkIntegration* ni, const char* url, const char* body, 
                                       size_t body_len, idcu_HttpResponse* response);
int  idcu_network_integration_http_delete(idcu_NetworkIntegration* ni, const char* url, idcu_HttpResponse* response);
int  idcu_network_integration_http_request(idcu_NetworkIntegration* ni, const idcu_HttpRequest* request, idcu_HttpResponse* response);

int  idcu_network_integration_register_http_route(idcu_NetworkIntegration* ni, const char* path, 
                                                   idcu_HttpMethod method, idcu_HttpHandler handler, void* user_data);
int  idcu_network_integration_unregister_http_route(idcu_NetworkIntegration* ni, const char* path, idcu_HttpMethod method);

int  idcu_network_integration_msg_publish(idcu_NetworkIntegration* ni, const char* topic, const char* message, size_t message_len);
int  idcu_network_integration_msg_subscribe(idcu_NetworkIntegration* ni, const char* topic, 
                                             idcu_MsgHandler handler, void* user_data);
int  idcu_network_integration_msg_unsubscribe(idcu_NetworkIntegration* ni, const char* topic, idcu_MsgHandler handler);

idcu_NetworkStatus idcu_network_integration_get_status(idcu_NetworkIntegration* ni);
int  idcu_network_integration_check_health(idcu_NetworkIntegration* ni);
int  idcu_network_integration_get_stats(idcu_NetworkIntegration* ni, uint64_t* bytes_sent, 
                                         uint64_t* bytes_received, uint64_t* request_count, uint64_t* error_count);
int  idcu_network_integration_reset_stats(idcu_NetworkIntegration* ni);

int  idcu_network_endpoint_init(idcu_NetworkEndpoint* endpoint, const char* name, const char* host, uint16_t port);
void idcu_network_endpoint_destroy(idcu_NetworkEndpoint* endpoint);

int  idcu_network_integration_enable_monitoring(idcu_NetworkIntegration* ni);
int  idcu_network_integration_disable_monitoring(idcu_NetworkIntegration* ni);

int  idcu_network_integration_register_health_check(idcu_NetworkIntegration* ni, const char* name, 
                                                      idcu_HealthCheckFunc func, void* user_data);
int  idcu_network_integration_unregister_health_check(idcu_NetworkIntegration* ni, const char* name);

int  idcu_network_integration_export_metrics(idcu_NetworkIntegration* ni, char* buffer, size_t buffer_size);
int  idcu_network_integration_get_info(idcu_NetworkIntegration* ni, char* buffer, size_t buffer_size);
int  idcu_network_integration_get_info_json(idcu_NetworkIntegration* ni, char* buffer, size_t buffer_size);

typedef struct
{
    idcu_Vector connections;
    idcu_Mutex lock;
    uint64_t total_connections;
    uint64_t active_connections;
    uint64_t failed_connections;
} idcu_NetworkConnectionManager;

int  idcu_network_connection_manager_init(idcu_NetworkConnectionManager* manager);
void idcu_network_connection_manager_destroy(idcu_NetworkConnectionManager* manager);
int  idcu_network_connection_manager_add(idcu_NetworkConnectionManager* manager, const idcu_NetworkEndpoint* endpoint);
int  idcu_network_connection_manager_remove(idcu_NetworkConnectionManager* manager, idcu_NetworkIntegrationId id);
int  idcu_network_connection_manager_connect(idcu_NetworkConnectionManager* manager, idcu_NetworkIntegrationId id);
int  idcu_network_connection_manager_disconnect(idcu_NetworkConnectionManager* manager, idcu_NetworkIntegrationId id);
int  idcu_network_connection_manager_get_stats(idcu_NetworkConnectionManager* manager, uint64_t* total, 
                                                uint64_t* active, uint64_t* failed);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `modules/network-integration/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(network-integration VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(network-integration STATIC
    src/idcu/network_integration/network_integration.c
    src/idcu/network_integration/connection_manager.c
)

target_include_directories(network-integration PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(network-integration PRIVATE
    idcu::common
    idcu::network
    idcu::conn-pool
    idcu::http-client
    idcu::http-server
    idcu::msgbus
    idcu::discovery
    idcu::healthcheck
    idcu::metrics
    idcu::log
    idcu::utils
)

add_library(idcu::network-integration ALIAS network-integration)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `modules/network-integration/module.yaml`：

```yaml
name: network-integration
version: 1.0.0
description: Network integration module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-network
  - idcu-conn-pool
  - idcu-http-client
  - idcu-http-server
  - idcu-msgbus
  - idcu-discovery
  - idcu-healthcheck
  - idcu-metrics
  - idcu-log
  - idcu-utils

build:
  type: cmake
  targets:
    - network-integration

headers:
  - idcu/network_integration/network_integration.h

features:
  - unified: Unified network interface
  - conn_pool: Connection pool integration
  - http_client: HTTP client integration
  - http_server: HTTP server integration
  - msgbus: Message bus integration
  - discovery: Node discovery integration
  - monitoring: Network monitoring
  - healthcheck: Network health check
  - config: Network config management
  - connection_manager: Connection manager

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `modules/network-integration/README.md`：

```markdown
# network-integration

IDCU Agent 的网络集成模块。

## 功能特性

- **统一接口**: 统一的网络接口
- **连接池**: 连接池集成
- **HTTP 客户端**: HTTP 客户端集成
- **HTTP 服务器**: HTTP 服务器集成
- **消息总线**: 消息总线集成
- **节点发现**: 节点发现集成
- **网络监控**: 网络监控
- **健康检查**: 网络健康检查
- **配置管理**: 网络配置管理
- **连接管理**: 连接管理器

## 快速开始

### 初始化网络集成

```c
#include "idcu/network_integration/network_integration.h"

idcu_NetworkIntegrationConfig config;
idcu_network_integration_config_init(&config);

config.enable_conn_pool = 1;
config.enable_http_client = 1;
config.enable_http_server = 1;
config.enable_msg_bus = 1;
config.enable_discovery = 1;
config.enable_monitoring = 1;
config.health_check_interval_ms = 5000;

idcu_NetworkIntegration ni;
idcu_network_integration_init(&ni, &config);
```

### 添加网络端点

```c
idcu_NetworkEndpoint endpoint;
idcu_network_endpoint_init(&endpoint, "api-server", "api.example.com", 443);
endpoint.enabled = 1;
endpoint.auto_reconnect = 1;
endpoint.reconnect_delay_ms = 5000;
endpoint.connect_timeout_ms = 10000;
endpoint.request_timeout_ms = 30000;

idcu_NetworkIntegrationId endpoint_id = idcu_network_integration_add_endpoint(&ni, &endpoint);
```

### 启动网络集成

```c
idcu_network_integration_start(&ni);
```

### HTTP 请求

```c
idcu_HttpResponse response;
idcu_http_response_init(&response);

if (idcu_network_integration_http_get(&ni, "https://api.example.com/data", &response) == IDCU_ERR_OK) {
    printf("Response: %.*s\n", (int)response.body_len, response.body);
}

idcu_http_response_destroy(&response);
```

### HTTP POST 请求

```c
const char* json_body = "{\"data\": \"test\"}";

idcu_HttpResponse response;
idcu_http_response_init(&response);

if (idcu_network_integration_http_post(&ni, "https://api.example.com/data", 
                                         json_body, strlen(json_body), &response) == IDCU_ERR_OK) {
    printf("Response: %.*s\n", (int)response.body_len, response.body);
}

idcu_http_response_destroy(&response);
```

### 注册 HTTP 路由

```c
int hello_handler(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data)
{
    idcu_http_response_set_status(response, IDCU_HTTP_STATUS_OK);
    idcu_http_response_set_header(response, "Content-Type", "text/plain");
    idcu_http_response_set_body(response, "Hello, World!", 13);
    return 0;
}

idcu_network_integration_register_http_route(&ni, "/hello", IDCU_HTTP_METHOD_GET, hello_handler, NULL);
```

### 消息总线 - 发布

```c
idcu_network_integration_msg_publish(&ni, "events", "Hello, Message!", 14);
```

### 消息总线 - 订阅

```c
void message_handler(const char* topic, const char* message, size_t message_len, void* user_data)
{
    printf("Received message on %s: %.*s\n", topic, (int)message_len, message);
}

idcu_network_integration_msg_subscribe(&ni, "events", message_handler, NULL);
```

### 获取网络状态

```c
idcu_NetworkStatus status = idcu_network_integration_get_status(&ni);
printf("Network status: %d\n", status);
```

### 健康检查

```c
if (idcu_network_integration_check_health(&ni) == IDCU_ERR_OK) {
    printf("Network is healthy\n");
} else {
    printf("Network is unhealthy\n");
}
```

### 获取统计

```c
uint64_t bytes_sent, bytes_received, request_count, error_count;
idcu_network_integration_get_stats(&ni, &bytes_sent, &bytes_received, &request_count, &error_count);

printf("Bytes sent: %" PRIu64 "\n", bytes_sent);
printf("Bytes received: %" PRIu64 "\n", bytes_received);
printf("Request count: %" PRIu64 "\n", request_count);
printf("Error count: %" PRIu64 "\n", error_count);
```

### 重置统计

```c
idcu_network_integration_reset_stats(&ni);
```

### 导出指标

```c
char metrics_buffer[4096];
idcu_network_integration_export_metrics(&ni, metrics_buffer, sizeof(metrics_buffer));
printf("%s\n", metrics_buffer);
```

### 获取信息

```c
char info_buffer[2048];
idcu_network_integration_get_info(&ni, info_buffer, sizeof(info_buffer));
printf("%s\n", info_buffer);

char json_buffer[4096];
idcu_network_integration_get_info_json(&ni, json_buffer, sizeof(json_buffer));
printf("%s\n", json_buffer);
```

### 使用连接管理器

```c
idcu_NetworkConnectionManager conn_manager;
idcu_network_connection_manager_init(&conn_manager);

idcu_network_connection_manager_add(&conn_manager, &endpoint);
idcu_network_connection_manager_connect(&conn_manager, endpoint_id);

uint64_t total, active, failed;
idcu_network_connection_manager_get_stats(&conn_manager, &total, &active, &failed);

idcu_network_connection_manager_destroy(&conn_manager);
```

### 停止网络集成

```c
idcu_network_integration_stop(&ni);
idcu_network_integration_destroy(&ni);
```

## 网络状态

| 状态 | 说明 |
|-----|------|
| DISCONNECTED | 已断开 |
| CONNECTING | 连接中 |
| CONNECTED | 已连接 |
| ERROR | 错误 |

## API 文档

详见 [include/idcu/network_integration/network_integration.h](include/idcu/network_integration/network_integration.h)
```

## 验证检查清单

- [ ] 网络集成头文件已创建
- [ ] 网络集成实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] HTTP 请求正常工作
- [ ] 消息总线正常工作
- [ ] 健康检查正常工作

## Git 提交

```bash
git add modules/network-integration/
git commit -m "feat: add network-integration module

- Add unified network interface
- Add connection pool integration
- Add HTTP client integration
- Add HTTP server integration
- Add message bus integration
- Add node discovery integration
- Add network monitoring
- Add network health check
- Add network config management
- Add connection manager
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 连接失败 | 网络不可达 | 检查网络连接和防火墙 |
| HTTP 请求超时 | 服务器响应慢 | 增加超时时间 |
| 消息未收到 | 订阅失败 | 检查订阅配置 |
