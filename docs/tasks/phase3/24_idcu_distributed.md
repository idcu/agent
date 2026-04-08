# 任务 3.24: idcu-distributed - 分布式节点支持

## 目标

创建分布式节点支持库，支持：
- 节点发现和注册
- 节点间通信
- 领导者选举
- 数据一致性
- 故障检测和恢复
- 消息广播和组播

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-distributed/include/idcu/distributed
mkdir -p libs/idcu-distributed/src/idcu/distributed
mkdir -p libs/idcu-distributed/tests
mkdir -p libs/idcu-distributed/examples
```

### 2. 创建分布式头文件 (distributed.h)

创建 `libs/idcu-distributed/include/idcu/distributed/distributed.h`：

```c
#ifndef IDCU_DISTRIBUTED_DISTRIBUTED_H
#define IDCU_DISTRIBUTED_DISTRIBUTED_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/lock.h"
#include "idcu/network/network.h"
#include "idcu/msgbus/msg_bus.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_NodeId;

typedef enum
{
    IDCU_NODE_STATE_UNKNOWN = 0,
    IDCU_NODE_STATE_OFFLINE,
    IDCU_NODE_STATE_ONLINE,
    IDCU_NODE_STATE_LEADER,
    IDCU_NODE_STATE_FOLLOWER,
    IDCU_NODE_STATE_CANDIDATE
} idcu_NodeState;

typedef struct
{
    idcu_NodeId id;
    char name[128];
    char address[256];
    uint16_t port;
    idcu_NodeState state;
    uint64_t last_heartbeat;
    uint64_t uptime;
    char metadata[1024];
} idcu_NodeInfo;

typedef enum
{
    IDCU_CONSISTENCY_STRONG = 0,
    IDCU_CONSISTENCY_EVENTUAL,
    IDCU_CONSISTENCY_QUORUM
} idcu_ConsistencyLevel;

typedef struct
{
    char id[128];
    char key[256];
    char value[4096];
    uint64_t version;
    uint64_t timestamp;
    idcu_NodeId origin_node;
} idcu_DistributedData;

typedef int (*idcu_NodeDiscoveryCallback)(idcu_NodeInfo* node, void* user_data);
typedef int (*idcu_LeaderElectionCallback)(idcu_NodeId leader_id, void* user_data);
typedef int (*idcu_DataSyncCallback)(idcu_DistributedData* data, void* user_data);
typedef int (*idcu_FailureCallback)(idcu_NodeId failed_node, void* user_data);

typedef struct
{
    char node_name[128];
    char bind_address[256];
    uint16_t bind_port;
    uint16_t discovery_port;
    uint64_t heartbeat_interval_ms;
    uint64_t failure_timeout_ms;
    idcu_ConsistencyLevel consistency_level;
    int enable_leader_election;
    int enable_data_sync;
} idcu_DistributedConfig;

typedef struct
{
    idcu_NodeId local_node_id;
    idcu_NodeInfo local_node;
    idcu_DistributedConfig config;
    
    idcu_Vector known_nodes;
    idcu_Vector online_nodes;
    idcu_Mutex nodes_lock;
    
    idcu_NodeId current_leader;
    int is_leader;
    
    idcu_NodeDiscoveryCallback discovery_callback;
    void* discovery_user_data;
    
    idcu_LeaderElectionCallback election_callback;
    void* election_user_data;
    
    idcu_DataSyncCallback data_callback;
    void* data_user_data;
    
    idcu_FailureCallback failure_callback;
    void* failure_user_data;
    
    idcu_NetworkServer* server;
    idcu_MsgBus* msg_bus;
    
    int initialized;
    int running;
} idcu_DistributedNode;

int  idcu_distributed_config_init(idcu_DistributedConfig* config);

int  idcu_distributed_node_init(idcu_DistributedNode* node, const idcu_DistributedConfig* config);
void idcu_distributed_node_destroy(idcu_DistributedNode* node);

int  idcu_distributed_node_start(idcu_DistributedNode* node);
int  idcu_distributed_node_stop(idcu_DistributedNode* node);

idcu_NodeId idcu_distributed_get_local_id(idcu_DistributedNode* node);
idcu_NodeInfo* idcu_distributed_get_local_info(idcu_DistributedNode* node);

int  idcu_distributed_set_metadata(idcu_DistributedNode* node, const char* metadata);
int  idcu_distributed_get_metadata(idcu_DistributedNode* node, char* buffer, size_t buffer_size);

int  idcu_distributed_register_node(idcu_DistributedNode* node, const idcu_NodeInfo* node_info);
int  idcu_distributed_unregister_node(idcu_DistributedNode* node, idcu_NodeId node_id);
idcu_NodeInfo* idcu_distributed_get_node(idcu_DistributedNode* node, idcu_NodeId node_id);
idcu_NodeInfo* idcu_distributed_get_node_by_name(idcu_DistributedNode* node, const char* name);

size_t idcu_distributed_get_node_count(idcu_DistributedNode* node);
int  idcu_distributed_get_all_nodes(idcu_DistributedNode* node, idcu_Vector* nodes);
int  idcu_distributed_get_online_nodes(idcu_DistributedNode* node, idcu_Vector* nodes);

int  idcu_distributed_set_discovery_callback(idcu_DistributedNode* node, 
                                              idcu_NodeDiscoveryCallback callback, 
                                              void* user_data);
int  idcu_distributed_set_election_callback(idcu_DistributedNode* node, 
                                             idcu_LeaderElectionCallback callback, 
                                             void* user_data);
int  idcu_distributed_set_data_callback(idcu_DistributedNode* node, 
                                         idcu_DataSyncCallback callback, 
                                         void* user_data);
int  idcu_distributed_set_failure_callback(idcu_DistributedNode* node, 
                                            idcu_FailureCallback callback, 
                                            void* user_data);

int  idcu_distributed_send_to(idcu_DistributedNode* node, idcu_NodeId target_id, 
                              const void* data, size_t data_size);
int  idcu_distributed_broadcast(idcu_DistributedNode* node, const void* data, size_t data_size);
int  idcu_distributed_multicast(idcu_DistributedNode* node, const idcu_Vector* target_ids, 
                                 const void* data, size_t data_size);

int  idcu_distributed_put_data(idcu_DistributedNode* node, const char* key, const char* value, 
                                idcu_ConsistencyLevel level);
int  idcu_distributed_get_data(idcu_DistributedNode* node, const char* key, char* value, 
                                size_t value_size, idcu_ConsistencyLevel level);
int  idcu_distributed_delete_data(idcu_DistributedNode* node, const char* key, 
                                   idcu_ConsistencyLevel level);

int  idcu_distributed_get_leader(idcu_DistributedNode* node, idcu_NodeId* leader_id);
int  idcu_distributed_is_leader(idcu_DistributedNode* node);
int  idcu_distributed_start_election(idcu_DistributedNode* node);

int  idcu_distributed_add_seed_node(idcu_DistributedNode* node, const char* address, uint16_t port);
int  idcu_distributed_discover_nodes(idcu_DistributedNode* node);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-distributed/module.yaml`：

```yaml
name: idcu-distributed
version: 1.0.0
description: Distributed node support library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-network
  - idcu-msgbus
  - idcu-log

build:
  type: cmake
  targets:
    - idcu-distributed

headers:
  - idcu/distributed/distributed.h

features:
  - node_discovery: Node discovery and registration
  - node_communication: Inter-node communication
  - leader_election: Leader election (Raft-like)
  - data_consistency: Data consistency with multiple levels
  - failure_detection: Failure detection and recovery
  - broadcast: Message broadcast and multicast
  - data_sync: Distributed data synchronization

testing:
  enabled: true
  framework: internal
```

### 4. 创建 README.md

创建 `libs/idcu-distributed/README.md`：

```markdown
# idcu-distributed

IDCU Agent 的分布式节点支持库。

## 功能特性

- **节点发现**: 自动发现和注册网络中的节点
- **节点通信**: 节点间可靠的通信机制
- **领导者选举**: 基于 Raft 算法的领导者选举
- **数据一致性**: 支持强一致性、最终一致性和仲裁一致性
- **故障检测**: 自动检测节点故障并恢复
- **消息广播**: 支持广播和组播消息
- **数据同步**: 分布式数据同步机制

## 快速开始

### 初始化分布式节点

```c
#include "idcu/distributed/distributed.h"

idcu_DistributedConfig config;
idcu_distributed_config_init(&config);

strncpy(config.node_name, "node-1", sizeof(config.node_name));
strncpy(config.bind_address, "0.0.0.0", sizeof(config.bind_address));
config.bind_port = 8080;
config.discovery_port = 8081;
config.heartbeat_interval_ms = 1000;
config.failure_timeout_ms = 5000;
config.consistency_level = IDCU_CONSISTENCY_QUORUM;
config.enable_leader_election = 1;
config.enable_data_sync = 1;

idcu_DistributedNode node;
idcu_distributed_node_init(&node, &config);
```

### 启动节点

```c
idcu_distributed_add_seed_node(&node, "192.168.1.100", 8081);
idcu_distributed_node_start(&node);
```

### 发送消息

```c
const char* message = "Hello from node-1";
idcu_distributed_broadcast(&node, message, strlen(message));
```

### 存储分布式数据

```c
idcu_distributed_put_data(&node, "service/config", "value=123", 
                          IDCU_CONSISTENCY_QUORUM);

char value[256];
idcu_distributed_get_data(&node, "service/config", value, sizeof(value),
                          IDCU_CONSISTENCY_QUORUM);
```

### 检查领导者状态

```c
if (idcu_distributed_is_leader(&node)) {
    printf("I am the leader!\n");
} else {
    idcu_NodeId leader_id;
    idcu_distributed_get_leader(&node, &leader_id);
    printf("Leader is: %" PRIu64 "\n", leader_id);
}
```

### 设置回调

```c
void on_node_discovered(idcu_NodeInfo* node_info, void* user_data)
{
    printf("Discovered node: %s\n", node_info->name);
}

void on_leader_elected(idcu_NodeId leader_id, void* user_data)
{
    printf("New leader elected: %" PRIu64 "\n", leader_id);
}

idcu_distributed_set_discovery_callback(&node, on_node_discovered, NULL);
idcu_distributed_set_election_callback(&node, on_leader_elected, NULL);
```

### 停止节点

```c
idcu_distributed_node_stop(&node);
idcu_distributed_node_destroy(&node);
```

## 一致性级别

| 级别 | 说明 |
|-----|------|
| STRONG | 强一致性，所有节点同步后返回 |
| EVENTUAL | 最终一致性，本地更新后立即返回 |
| QUORUM | 仲裁一致性，多数节点同步后返回 |

## 节点状态

| 状态 | 说明 |
|-----|------|
| UNKNOWN | 未知状态 |
| OFFLINE | 离线 |
| ONLINE | 在线 |
| LEADER | 领导者 |
| FOLLOWER | 跟随者 |
| CANDIDATE | 候选者 |

## API 文档

详见 [include/idcu/distributed/distributed.h](include/idcu/distributed/distributed.h)
```

## 验证检查清单

- [ ] 分布式头文件已创建
- [ ] 分布式实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 节点可以正常启动和停止
- [ ] 节点发现功能正常工作
- [ ] 消息广播功能正常工作

## Git 提交

```bash
git add libs/idcu-distributed/
git commit -m "feat: add idcu-distributed library

- Add node discovery and registration
- Add inter-node communication
- Add leader election (Raft-like)
- Add data consistency with multiple levels
- Add failure detection and recovery
- Add message broadcast and multicast
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 节点无法发现其他节点 | 网络问题或种子节点配置错误 | 检查网络连接和种子节点配置 |
| 领导者选举失败 | 节点数量不足或网络分区 | 确保至少 3 个节点，检查网络 |
| 数据不同步 | 一致性级别设置或网络延迟 | 调整一致性级别，检查网络 |
