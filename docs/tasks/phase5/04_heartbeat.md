# 任务 5.4: heartbeat - 心跳模块

## 目标

创建心跳模块，支持：
- 定时心跳发送
- 心跳接收和验证
- 节点状态监控
- 心跳超时检测
- 自动重连
- 心跳历史记录
- 心跳统计
- 多协议支持
- 自定义心跳数据

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/heartbeat/include/idcu/heartbeat
mkdir -p modules/heartbeat/src/idcu/heartbeat
mkdir -p modules/heartbeat/tests
mkdir -p modules/heartbeat/examples
```

### 2. 创建心跳模块头文件 (heartbeat.h)

创建 `modules/heartbeat/include/idcu/heartbeat/heartbeat.h`：

```c
#ifndef IDCU_HEARTBEAT_HEARTBEAT_H
#define IDCU_HEARTBEAT_HEARTBEAT_H

#include "idcu/common/error_code.h"
#include "idcu/network/network.h"
#include "idcu/msgbus/msg_bus.h"
#include "idcu/metrics/metrics.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_HeartbeatNodeId;

typedef enum
{
    IDCU_HEARTBEAT_STATUS_UNKNOWN = 0,
    IDCU_HEARTBEAT_STATUS_ALIVE,
    IDCU_HEARTBEAT_STATUS_WARNING,
    IDCU_HEARTBEAT_STATUS_DEAD,
    IDCU_HEARTBEAT_STATUS_OFFLINE
} idcu_HeartbeatStatus;

typedef enum
{
    IDCU_HEARTBEAT_PROTOCOL_TCP = 0,
    IDCU_HEARTBEAT_PROTOCOL_UDP,
    IDCU_HEARTBEAT_PROTOCOL_HTTP,
    IDCU_HEARTBEAT_PROTOCOL_MSGBUS
} idcu_HeartbeatProtocol;

typedef struct
{
    uint64_t id;
    uint64_t timestamp;
    uint64_t sequence;
    char node_id[128];
    char node_name[128];
    char node_type[64];
    char data[2048];
    uint32_t checksum;
} idcu_HeartbeatPacket;

typedef struct
{
    idcu_HeartbeatNodeId id;
    char node_id[128];
    char node_name[128];
    char node_type[64];
    char address[256];
    uint16_t port;
    
    idcu_HeartbeatStatus status;
    idcu_HeartbeatProtocol protocol;
    
    uint64_t last_heartbeat_at;
    uint64_t first_heartbeat_at;
    uint64_t heartbeat_count;
    uint64_t missed_count;
    
    uint64_t interval_ms;
    uint64_t timeout_ms;
    uint64_t warning_threshold_ms;
    
    idcu_Vector history;
    size_t max_history;
    
    int auto_reconnect;
    uint64_t reconnect_delay_ms;
    int reconnect_attempts;
    int max_reconnect_attempts;
    
    char custom_data[2048];
} idcu_HeartbeatNode;

typedef void (*idcu_HeartbeatCallback)(idcu_HeartbeatNode* node, idcu_HeartbeatStatus old_status, 
                                        idcu_HeartbeatStatus new_status, void* user_data);
typedef void (*idcu_HeartbeatPacketCallback)(const idcu_HeartbeatPacket* packet, void* user_data);

typedef struct
{
    idcu_Vector nodes;
    idcu_HashMap nodes_by_id;
    idcu_HashMap nodes_by_address;
    idcu_Mutex lock;
    
    idcu_HeartbeatNode* local_node;
    char local_node_id[128];
    char local_node_name[128];
    char local_node_type[64];
    
    idcu_Thread sender_thread;
    idcu_Thread receiver_thread;
    idcu_Thread monitor_thread;
    
    idcu_Socket* tcp_socket;
    idcu_Socket* udp_socket;
    idcu_MsgBus* msg_bus;
    
    idcu_Vector status_callbacks;
    idcu_Vector packet_callbacks;
    
    idcu_Counter* heartbeat_sent_counter;
    idcu_Counter* heartbeat_received_counter;
    idcu_Counter* heartbeat_missed_counter;
    idcu_Gauge* alive_nodes_gauge;
    
    uint64_t default_interval_ms;
    uint64_t default_timeout_ms;
    uint64_t default_warning_threshold_ms;
    
    int running;
    int initialized;
} idcu_HeartbeatManager;

typedef struct
{
    char local_node_id[128];
    char local_node_name[128];
    char local_node_type[64];
    char local_address[256];
    uint16_t local_port;
    
    idcu_HeartbeatProtocol default_protocol;
    uint64_t default_interval_ms;
    uint64_t default_timeout_ms;
    uint64_t default_warning_threshold_ms;
    
    int enable_metrics;
    int enable_auto_reconnect;
    uint64_t default_reconnect_delay_ms;
    int default_max_reconnect_attempts;
} idcu_HeartbeatManagerConfig;

int  idcu_heartbeat_manager_config_init(idcu_HeartbeatManagerConfig* config);

int  idcu_heartbeat_manager_init(idcu_HeartbeatManager* hb, const idcu_HeartbeatManagerConfig* config);
void idcu_heartbeat_manager_destroy(idcu_HeartbeatManager* hb);
int  idcu_heartbeat_manager_start(idcu_HeartbeatManager* hb);
void idcu_heartbeat_manager_stop(idcu_HeartbeatManager* hb);

idcu_HeartbeatNodeId idcu_heartbeat_manager_add_node(idcu_HeartbeatManager* hb, const idcu_HeartbeatNode* node);
int  idcu_heartbeat_manager_remove_node(idcu_HeartbeatManager* hb, idcu_HeartbeatNodeId id);
idcu_HeartbeatNode* idcu_heartbeat_manager_get_node(idcu_HeartbeatManager* hb, idcu_HeartbeatNodeId id);
idcu_HeartbeatNode* idcu_heartbeat_manager_get_node_by_address(idcu_HeartbeatManager* hb, const char* address, uint16_t port);
int  idcu_heartbeat_manager_enable_node(idcu_HeartbeatManager* hb, idcu_HeartbeatNodeId id);
int  idcu_heartbeat_manager_disable_node(idcu_HeartbeatManager* hb, idcu_HeartbeatNodeId id);

int  idcu_heartbeat_manager_add_status_callback(idcu_HeartbeatManager* hb, idcu_HeartbeatCallback callback, void* user_data);
int  idcu_heartbeat_manager_remove_status_callback(idcu_HeartbeatManager* hb, idcu_HeartbeatCallback callback);
int  idcu_heartbeat_manager_add_packet_callback(idcu_HeartbeatManager* hb, idcu_HeartbeatPacketCallback callback, void* user_data);
int  idcu_heartbeat_manager_remove_packet_callback(idcu_HeartbeatManager* hb, idcu_HeartbeatPacketCallback callback);

int  idcu_heartbeat_manager_set_msgbus(idcu_HeartbeatManager* hb, idcu_MsgBus* msg_bus);
int  idcu_heartbeat_manager_set_metrics(idcu_HeartbeatManager* hb, idcu_Counter* sent, idcu_Counter* received, 
                                           idcu_Counter* missed, idcu_Gauge* alive);

int  idcu_heartbeat_manager_send_heartbeat(idcu_HeartbeatManager* hb, idcu_HeartbeatNodeId node_id);
int  idcu_heartbeat_manager_send_all(idcu_HeartbeatManager* hb);
int  idcu_heartbeat_manager_broadcast_heartbeat(idcu_HeartbeatManager* hb);

int  idcu_heartbeat_manager_get_alive_nodes(idcu_HeartbeatManager* hb, idcu_Vector* nodes);
int  idcu_heartbeat_manager_get_dead_nodes(idcu_HeartbeatManager* hb, idcu_Vector* nodes);
int  idcu_heartbeat_manager_get_all_nodes(idcu_HeartbeatManager* hb, idcu_Vector* nodes);
size_t idcu_heartbeat_manager_node_count(idcu_HeartbeatManager* hb);
size_t idcu_heartbeat_manager_alive_count(idcu_HeartbeatManager* hb);

int  idcu_heartbeat_node_init(idcu_HeartbeatNode* node, const char* node_id, const char* node_name);
void idcu_heartbeat_node_destroy(idcu_HeartbeatNode* node);
int  idcu_heartbeat_node_set_address(idcu_HeartbeatNode* node, const char* address, uint16_t port);
int  idcu_heartbeat_node_set_protocol(idcu_HeartbeatNode* node, idcu_HeartbeatProtocol protocol);
int  idcu_heartbeat_node_set_interval(idcu_HeartbeatNode* node, uint64_t interval_ms);
int  idcu_heartbeat_node_set_timeout(idcu_HeartbeatNode* node, uint64_t timeout_ms);
int  idcu_heartbeat_node_set_warning_threshold(idcu_HeartbeatNode* node, uint64_t threshold_ms);
int  idcu_heartbeat_node_set_auto_reconnect(idcu_HeartbeatNode* node, int enabled, uint64_t delay_ms, int max_attempts);
int  idcu_heartbeat_node_set_custom_data(idcu_HeartbeatNode* node, const char* data);
int  idcu_heartbeat_node_get_status(idcu_HeartbeatNode* node, idcu_HeartbeatStatus* status);
uint64_t idcu_heartbeat_node_get_uptime(idcu_HeartbeatNode* node);

int  idcu_heartbeat_packet_init(idcu_HeartbeatPacket* packet);
void idcu_heartbeat_packet_destroy(idcu_HeartbeatPacket* packet);
int  idcu_heartbeat_packet_set_node(idcu_HeartbeatPacket* packet, const char* node_id, const char* node_name, const char* node_type);
int  idcu_heartbeat_packet_set_data(idcu_HeartbeatPacket* packet, const char* data);
int  idcu_heartbeat_packet_serialize(const idcu_HeartbeatPacket* packet, char* buffer, size_t buffer_size, size_t* written);
int  idcu_heartbeat_packet_deserialize(idcu_HeartbeatPacket* packet, const char* buffer, size_t buffer_size);
uint32_t idcu_heartbeat_packet_calculate_checksum(const idcu_HeartbeatPacket* packet);
int  idcu_heartbeat_packet_verify_checksum(const idcu_HeartbeatPacket* packet);

const char* idcu_heartbeat_status_to_string(idcu_HeartbeatStatus status);
const char* idcu_heartbeat_protocol_to_string(idcu_HeartbeatProtocol protocol);

int  idcu_heartbeat_manager_get_stats(idcu_HeartbeatManager* hb, uint64_t* total_sent, uint64_t* total_received, 
                                        uint64_t* total_missed, size_t* alive_count);
int  idcu_heartbeat_manager_reset_stats(idcu_HeartbeatManager* hb);

int  idcu_heartbeat_manager_export_json(idcu_HeartbeatManager* hb, char* buffer, size_t buffer_size);
int  idcu_heartbeat_manager_get_info(idcu_HeartbeatManager* hb, char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `modules/heartbeat/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(heartbeat VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(heartbeat STATIC
    src/idcu/heartbeat/heartbeat.c
    src/idcu/heartbeat/heartbeat_packet.c
    src/idcu/heartbeat/heartbeat_node.c
)

target_include_directories(heartbeat PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(heartbeat PRIVATE
    idcu::common
    idcu::network
    idcu::msgbus
    idcu::metrics
    idcu::utils
    idcu::log
)

add_library(idcu::heartbeat ALIAS heartbeat)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `modules/heartbeat/module.yaml`：

```yaml
name: heartbeat
version: 1.0.0
description: Heartbeat module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-network
  - idcu-msgbus
  - idcu-metrics
  - idcu-utils
  - idcu-log

build:
  type: cmake
  targets:
    - heartbeat

headers:
  - idcu/heartbeat/heartbeat.h

features:
  - send: Periodic heartbeat sending
  - receive: Heartbeat receiving and validation
  - monitor: Node status monitoring
  - timeout: Heartbeat timeout detection
  - reconnect: Auto reconnect
  - history: Heartbeat history
  - stats: Heartbeat statistics
  - multi_protocol: Multi-protocol support
  - custom: Custom heartbeat data

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `modules/heartbeat/README.md`：

```markdown
# heartbeat

IDCU Agent 的心跳模块。

## 功能特性

- **定时发送**: 定时心跳发送
- **接收验证**: 心跳接收和验证
- **状态监控**: 节点状态监控
- **超时检测**: 心跳超时检测
- **自动重连**: 自动重连
- **历史记录**: 心跳历史记录
- **统计信息**: 心跳统计
- **多协议支持**: 多协议支持
- **自定义数据**: 自定义心跳数据

## 快速开始

### 初始化心跳管理器

```c
#include "idcu/heartbeat/heartbeat.h"

idcu_HeartbeatManagerConfig config;
idcu_heartbeat_manager_config_init(&config);

strncpy(config.local_node_id, "node-001", sizeof(config.local_node_id));
strncpy(config.local_node_name, "Main Server", sizeof(config.local_node_name));
strncpy(config.local_node_type, "server", sizeof(config.local_node_type));
config.default_protocol = IDCU_HEARTBEAT_PROTOCOL_TCP;
config.default_interval_ms = 5000;
config.default_timeout_ms = 15000;
config.default_warning_threshold_ms = 10000;
config.enable_metrics = 1;
config.enable_auto_reconnect = 1;

idcu_HeartbeatManager hb;
idcu_heartbeat_manager_init(&hb, &config);
```

### 添加节点

```c
idcu_HeartbeatNode node;
idcu_heartbeat_node_init(&node, "node-002", "Worker 1");
idcu_heartbeat_node_set_address(&node, "192.168.1.100", 8888);
idcu_heartbeat_node_set_protocol(&node, IDCU_HEARTBEAT_PROTOCOL_TCP);
idcu_heartbeat_node_set_interval(&node, 5000);
idcu_heartbeat_node_set_timeout(&node, 15000);
idcu_heartbeat_node_set_auto_reconnect(&node, 1, 3000, 5);

idcu_HeartbeatNodeId node_id = idcu_heartbeat_manager_add_node(&hb, &node);
```

### 添加状态回调

```c
void status_changed(idcu_HeartbeatNode* node, idcu_HeartbeatStatus old_status,
                    idcu_HeartbeatStatus new_status, void* user_data)
{
    printf("Node %s status changed: %s -> %s\n",
           node->node_name,
           idcu_heartbeat_status_to_string(old_status),
           idcu_heartbeat_status_to_string(new_status));
}

idcu_heartbeat_manager_add_status_callback(&hb, status_changed, NULL);
```

### 添加数据包回调

```c
void packet_received(const idcu_HeartbeatPacket* packet, void* user_data)
{
    printf("Received heartbeat from %s (seq: %" PRIu64 ")\n",
           packet->node_name, packet->sequence);
}

idcu_heartbeat_manager_add_packet_callback(&hb, packet_received, NULL);
```

### 设置消息总线

```c
idcu_heartbeat_manager_set_msgbus(&hb, msg_bus);
```

### 设置指标

```c
idcu_heartbeat_manager_set_metrics(&hb, sent_counter, received_counter, missed_counter, alive_gauge);
```

### 启动心跳管理器

```c
idcu_heartbeat_manager_start(&hb);
```

### 手动发送心跳

```c
idcu_heartbeat_manager_send_heartbeat(&hb, node_id);
idcu_heartbeat_manager_send_all(&hb);
idcu_heartbeat_manager_broadcast_heartbeat(&hb);
```

### 获取节点

```c
idcu_HeartbeatNode* node = idcu_heartbeat_manager_get_node(&hb, node_id);
idcu_HeartbeatNode* node_by_addr = idcu_heartbeat_manager_get_node_by_address(&hb, "192.168.1.100", 8888);
```

### 获取节点状态

```c
idcu_HeartbeatStatus status;
idcu_heartbeat_node_get_status(node, &status);
printf("Node status: %s\n", idcu_heartbeat_status_to_string(status));
```

### 获取节点运行时间

```c
uint64_t uptime = idcu_heartbeat_node_get_uptime(node);
printf("Node uptime: %" PRIu64 " ms\n", uptime);
```

### 获取活跃节点

```c
idcu_Vector alive_nodes;
idcu_vector_init(&alive_nodes, sizeof(idcu_HeartbeatNodeId));

idcu_heartbeat_manager_get_alive_nodes(&hb, &alive_nodes);

for (size_t i = 0; i < alive_nodes.count; i++) {
    idcu_HeartbeatNodeId* id_ptr = (idcu_HeartbeatNodeId*)idcu_vector_get(&alive_nodes, i);
    printf("Alive node ID: %" PRIu64 "\n", *id_ptr);
}

idcu_vector_destroy(&alive_nodes);
```

### 获取所有节点

```c
idcu_Vector all_nodes;
idcu_vector_init(&all_nodes, sizeof(idcu_HeartbeatNodeId));

idcu_heartbeat_manager_get_all_nodes(&hb, &all_nodes);

printf("Total nodes: %zu\n", idcu_heartbeat_manager_node_count(&hb));
printf("Alive nodes: %zu\n", idcu_heartbeat_manager_alive_count(&hb));

idcu_vector_destroy(&all_nodes);
```

### 获取统计

```c
uint64_t total_sent, total_received, total_missed;
size_t alive_count;

idcu_heartbeat_manager_get_stats(&hb, &total_sent, &total_received, &total_missed, &alive_count);

printf("Total sent: %" PRIu64 "\n", total_sent);
printf("Total received: %" PRIu64 "\n", total_received);
printf("Total missed: %" PRIu64 "\n", total_missed);
printf("Alive count: %zu\n", alive_count);
```

### 重置统计

```c
idcu_heartbeat_manager_reset_stats(&hb);
```

### 禁用节点

```c
idcu_heartbeat_manager_disable_node(&hb, node_id);
```

### 启用节点

```c
idcu_heartbeat_manager_enable_node(&hb, node_id);
```

### 移除节点

```c
idcu_heartbeat_manager_remove_node(&hb, node_id);
```

### 创建心跳包

```c
idcu_HeartbeatPacket packet;
idcu_heartbeat_packet_init(&packet);
idcu_heartbeat_packet_set_node(&packet, "node-001", "Main Server", "server");
idcu_heartbeat_packet_set_data(&packet, "{\"load\": 0.5}");
```

### 序列化心跳包

```c
char buffer[4096];
size_t written;

idcu_heartbeat_packet_serialize(&packet, buffer, sizeof(buffer), &written);
```

### 反序列化心跳包

```c
idcu_HeartbeatPacket received_packet;
idcu_heartbeat_packet_init(&received_packet);

idcu_heartbeat_packet_deserialize(&received_packet, buffer, written);
```

### 验证校验和

```c
if (idcu_heartbeat_packet_verify_checksum(&received_packet)) {
    printf("Checksum verified\n");
} else {
    printf("Checksum invalid\n");
}
```

### 导出信息

```c
char info_buffer[4096];
idcu_heartbeat_manager_get_info(&hb, info_buffer, sizeof(info_buffer));
printf("%s\n", info_buffer);

char json_buffer[8192];
idcu_heartbeat_manager_export_json(&hb, json_buffer, sizeof(json_buffer));
printf("%s\n", json_buffer);
```

### 停止心跳管理器

```c
idcu_heartbeat_manager_stop(&hb);
idcu_heartbeat_manager_destroy(&hb);
```

## 节点状态

| 状态 | 说明 |
|-----|------|
| UNKNOWN | 未知 |
| ALIVE | 活跃 |
| WARNING | 警告 |
| DEAD | 死亡 |
| OFFLINE | 离线 |

## 协议类型

| 类型 | 说明 |
|-----|------|
| TCP | TCP 协议 |
| UDP | UDP 协议 |
| HTTP | HTTP 协议 |
| MSGBUS | 消息总线 |

## API 文档

详见 [include/idcu/heartbeat/heartbeat.h](include/idcu/heartbeat/heartbeat.h)
```

## 验证检查清单

- [ ] 心跳模块头文件已创建
- [ ] 心跳模块实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以发送和接收心跳
- [ ] 节点状态监控正常工作
- [ ] 心跳超时检测正常工作

## Git 提交

```bash
git add modules/heartbeat/
git commit -m "feat: add heartbeat module

- Add periodic heartbeat sending
- Add heartbeat receiving and validation
- Add node status monitoring
- Add heartbeat timeout detection
- Add auto reconnect
- Add heartbeat history
- Add heartbeat statistics
- Add multi-protocol support
- Add custom heartbeat data
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 心跳未发送 | 节点未启用 | 确保节点已启用 |
| 心跳超时 | 网络延迟 | 增加超时时间 |
| 节点离线 | 连接断开 | 检查网络连接 |
