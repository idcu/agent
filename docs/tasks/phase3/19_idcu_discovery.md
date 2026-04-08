# 任务 3.19: idcu-discovery - 节点发现库

## 目标

创建节点发现库，支持：
- UDP 广播发现
- 多播发现
- 服务注册和发现
- 节点信息交换
- 心跳检测
- 节点列表管理

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-discovery/include/idcu/discovery
mkdir -p libs/idcu-discovery/src/idcu/discovery
mkdir -p libs/idcu-discovery/tests
mkdir -p libs/idcu-discovery/examples
```

### 2. 创建节点发现头文件 (discovery.h)

创建 `libs/idcu-discovery/include/idcu/discovery/discovery.h`：

```c
#ifndef IDCU_DISCOVERY_DISCOVERY_H
#define IDCU_DISCOVERY_DISCOVERY_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include "idcu/common/thread.h"
#include "idcu/network/network.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_NodeId;

typedef enum
{
    IDCU_NODE_STATUS_UNKNOWN = 0,
    IDCU_NODE_STATUS_ONLINE,
    IDCU_NODE_STATUS_OFFLINE,
    IDCU_NODE_STATUS_SUSPECT
} idcu_NodeStatus;

typedef struct
{
    idcu_NodeId id;
    char name[128];
    char address[256];
    uint16_t port;
    idcu_NodeStatus status;
    char tags[512];
    char metadata[1024];
    uint64_t first_seen_at;
    uint64_t last_seen_at;
    uint64_t last_heartbeat_at;
    uint64_t heartbeat_interval_ms;
} idcu_DiscoveredNode;

typedef struct
{
    idcu_NodeId local_node_id;
    char local_node_name[128];
    char local_address[256];
    uint16_t local_port;
    char local_tags[512];
    char local_metadata[1024];
} idcu_LocalNodeConfig;

typedef struct
{
    idcu_Vector nodes;
    idcu_HashMap nodes_by_id;
    idcu_Mutex lock;
    
    idcu_UdpSocket broadcast_socket;
    idcu_UdpSocket multicast_socket;
    
    idcu_Thread listener_thread;
    idcu_Thread heartbeat_thread;
    
    char multicast_group[64];
    uint16_t multicast_port;
    uint16_t broadcast_port;
    
    uint64_t heartbeat_interval_ms;
    uint64_t node_timeout_ms;
    
    idcu_LocalNodeConfig local_config;
    
    int running;
    int initialized;
    
    void (*on_node_discovered)(const idcu_DiscoveredNode* node, void* user_data);
    void (*on_node_lost)(const idcu_DiscoveredNode* node, void* user_data);
    void (*on_node_updated)(const idcu_DiscoveredNode* node, void* user_data);
    void* callback_user_data;
} idcu_Discovery;

typedef struct
{
    char multicast_group[64];
    uint16_t multicast_port;
    uint16_t broadcast_port;
    uint64_t heartbeat_interval_ms;
    uint64_t node_timeout_ms;
} idcu_DiscoveryConfig;

int  idcu_discovery_config_init(idcu_DiscoveryConfig* config);

int  idcu_discovery_init(idcu_Discovery* discovery, const idcu_DiscoveryConfig* config, const idcu_LocalNodeConfig* local_config);
void idcu_discovery_destroy(idcu_Discovery* discovery);
int  idcu_discovery_start(idcu_Discovery* discovery);
void idcu_discovery_stop(idcu_Discovery* discovery);

int  idcu_discovery_set_callbacks(idcu_Discovery* discovery,
                                   void (*on_discovered)(const idcu_DiscoveredNode* node, void* user_data),
                                   void (*on_lost)(const idcu_DiscoveredNode* node, void* user_data),
                                   void (*on_updated)(const idcu_DiscoveredNode* node, void* user_data),
                                   void* user_data);

idcu_DiscoveredNode* idcu_discovery_get_node(idcu_Discovery* discovery, idcu_NodeId node_id);
idcu_DiscoveredNode* idcu_discovery_get_node_by_name(idcu_Discovery* discovery, const char* name);
size_t idcu_discovery_get_node_count(idcu_Discovery* discovery);
int  idcu_discovery_get_all_nodes(idcu_Discovery* discovery, idcu_Vector* nodes);
int  idcu_discovery_get_online_nodes(idcu_Discovery* discovery, idcu_Vector* nodes);

int  idcu_discovery_broadcast_presence(idcu_Discovery* discovery);
int  idcu_discovery_multicast_presence(idcu_Discovery* discovery);

int  idcu_discovery_add_tag(idcu_Discovery* discovery, const char* tag);
int  idcu_discovery_remove_tag(idcu_Discovery* discovery, const char* tag);
int  idcu_discovery_set_metadata(idcu_Discovery* discovery, const char* key, const char* value);

int  idcu_discovered_node_init(idcu_DiscoveredNode* node);
void idcu_discovered_node_destroy(idcu_DiscoveredNode* node);
int  idcu_discovered_node_copy(idcu_DiscoveredNode* dest, const idcu_DiscoveredNode* src);
int  idcu_discovered_node_to_json(const idcu_DiscoveredNode* node, char* buffer, size_t buffer_size);
int  idcu_discovered_node_from_json(idcu_DiscoveredNode* node, const char* json);

int  idcu_local_node_config_init(idcu_LocalNodeConfig* config);
void idcu_local_node_config_destroy(idcu_LocalNodeConfig* config);
int  idcu_local_node_config_set_name(idcu_LocalNodeConfig* config, const char* name);
int  idcu_local_node_config_set_address(idcu_LocalNodeConfig* config, const char* address);
int  idcu_local_node_config_set_port(idcu_LocalNodeConfig* config, uint16_t port);
int  idcu_local_node_config_add_tag(idcu_LocalNodeConfig* config, const char* tag);
int  idcu_local_node_config_set_metadata(idcu_LocalNodeConfig* config, const char* key, const char* value);

idcu_NodeId idcu_discovery_generate_node_id(void);

const char* idcu_node_status_to_string(idcu_NodeStatus status);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-discovery/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-discovery VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-discovery STATIC
    src/idcu/discovery/discovery.c
)

target_include_directories(idcu-discovery PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-discovery PRIVATE
    idcu::common
    idcu::network
    idcu::json
    idcu::log
)

add_library(idcu::discovery ALIAS idcu-discovery)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-discovery/module.yaml`：

```yaml
name: idcu-discovery
version: 1.0.0
description: Node discovery library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-network
  - idcu-json
  - idcu-log

build:
  type: cmake
  targets:
    - idcu-discovery

headers:
  - idcu/discovery/discovery.h

features:
  - udp_broadcast: UDP broadcast discovery
  - multicast: Multicast discovery
  - service_registry: Service registration and discovery
  - node_info: Node information exchange
  - heartbeat: Heartbeat detection
  - node_list: Node list management
  - callbacks: Node discovery/lost/updated callbacks

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `libs/idcu-discovery/README.md`：

```markdown
# idcu-discovery

IDCU Agent 的节点发现库。

## 功能特性

- **UDP 广播**: UDP 广播发现
- **多播**: 多播发现
- **服务注册**: 服务注册和发现
- **节点信息**: 节点信息交换
- **心跳检测**: 心跳检测
- **节点列表**: 节点列表管理
- **回调**: 节点发现/丢失/更新回调

## 快速开始

### 初始化发现服务

```c
#include "idcu/discovery/discovery.h"

idcu_LocalNodeConfig local_config;
idcu_local_node_config_init(&local_config);

idcu_local_node_config_set_name(&local_config, "my-node");
idcu_local_node_config_set_address(&local_config, "192.168.1.100");
idcu_local_node_config_set_port(&local_config, 8080);
idcu_local_node_config_add_tag(&local_config, "worker");
idcu_local_node_config_set_metadata(&local_config, "region", "us-east-1");

idcu_DiscoveryConfig config;
idcu_discovery_config_init(&config);

strncpy(config.multicast_group, "239.255.0.1", sizeof(config.multicast_group));
config.multicast_port = 9999;
config.broadcast_port = 9998;
config.heartbeat_interval_ms = 5000;
config.node_timeout_ms = 15000;

idcu_Discovery discovery;
idcu_discovery_init(&discovery, &config, &local_config);
```

### 设置回调

```c
void on_node_discovered(const idcu_DiscoveredNode* node, void* user_data)
{
    printf("Discovered node: %s (%s:%d)\n", 
           node->name, node->address, node->port);
}

void on_node_lost(const idcu_DiscoveredNode* node, void* user_data)
{
    printf("Lost node: %s\n", node->name);
}

void on_node_updated(const idcu_DiscoveredNode* node, void* user_data)
{
    printf("Updated node: %s\n", node->name);
}

idcu_discovery_set_callbacks(&discovery, 
                              on_node_discovered, 
                              on_node_lost, 
                              on_node_updated, 
                              NULL);
```

### 启动发现服务

```c
idcu_discovery_start(&discovery);
```

### 获取发现的节点

```c
idcu_Vector online_nodes;
idcu_vector_init(&online_nodes, sizeof(idcu_DiscoveredNode));

idcu_discovery_get_online_nodes(&discovery, &online_nodes);

for (size_t i = 0; i < online_nodes.count; i++) {
    idcu_DiscoveredNode* node = (idcu_DiscoveredNode*)idcu_vector_get(&online_nodes, i);
    printf("Node: %s, Status: %s\n", 
           node->name, idcu_node_status_to_string(node->status));
}

idcu_vector_destroy(&online_nodes);
```

### 通过 ID 获取节点

```c
idcu_DiscoveredNode* node = idcu_discovery_get_node(&discovery, node_id);
if (node) {
    printf("Node: %s\n", node->name);
}
```

### 通过名称获取节点

```c
idcu_DiscoveredNode* node = idcu_discovery_get_node_by_name(&discovery, "other-node");
```

### 更新本地节点信息

```c
idcu_discovery_add_tag(&discovery, "new-tag");
idcu_discovery_set_metadata(&discovery, "version", "2.0.0");
```

### 手动广播存在

```c
idcu_discovery_broadcast_presence(&discovery);
idcu_discovery_multicast_presence(&discovery);
```

### 停止发现服务

```c
idcu_discovery_stop(&discovery);
idcu_discovery_destroy(&discovery);
```

## 节点状态

| 状态 | 说明 |
|-----|------|
| UNKNOWN | 未知 |
| ONLINE | 在线 |
| OFFLINE | 离线 |
| SUSPECT | 可疑 |

## API 文档

详见 [include/idcu/discovery/discovery.h](include/idcu/discovery/discovery.h)
```

## 验证检查清单

- [ ] 节点发现头文件已创建
- [ ] 节点发现实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以发现其他节点
- [ ] 心跳检测正常工作
- [ ] 回调可以正常触发

## Git 提交

```bash
git add libs/idcu-discovery/
git commit -m "feat: add idcu-discovery library

- Add UDP broadcast discovery
- Add multicast discovery
- Add service registration and discovery
- Add node information exchange
- Add heartbeat detection
- Add node list management
- Add node discovery/lost/updated callbacks
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 无法发现节点 | 网络隔离 | 检查网络配置和防火墙 |
| 节点频繁掉线 | 心跳超时太短 | 增加 node_timeout_ms |
| 多播不工作 | 路由器不支持 | 使用 UDP 广播替代 |
