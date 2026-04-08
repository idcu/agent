# 任务 3.19: idcu-discovery - 节点发现库

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建节点发现库，支持 UDP 广播发现、多播发现、服务注册和发现、节点信息交换、心跳检测、节点列表管理，支持节点发现延迟 ≤ 5 秒、心跳间隔可配置（默认 5 秒）、支持 100+ 节点并发发现的性能要求。

### 1.2 不做什么
- 不实现 DNS 服务发现
- 不实现基于 KV 存储的服务发现（如 etcd、consul）
- 不实现跨网段的节点发现（需网络层路由支持）
- 不实现节点间的认证和加密

### 1.3 输入
- 本地节点配置（节点名称、地址、端口、标签、元数据）
- 发现配置（多播组、多播端口、广播端口、心跳间隔、节点超时）
- 回调函数（节点发现、节点丢失、节点更新）

### 1.4 输出
- 发现的节点列表
- 节点状态更新通知
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 基础库已可用
- idcu-network 网络库已可用
- idcu-json 库已可用
- idcu-log 库已可用
- phase3 已完成前 18 个任务

---

## 2. 技术实现方案

### 2.1 核心选型
- **发现协议**: UDP 广播 + UDP 多播（双模式）
- **心跳机制**: 定时发送心跳包
- **节点管理**: 哈希表 + 向量存储节点信息
- **并发安全**: 互斥锁保护共享数据
- **多线程**: 独立线程处理监听和心跳

### 2.2 核心逻辑
```
节点发现流程：
1. 初始化本地节点配置
2. 创建 UDP 广播和多播 Socket
3. 启动监听线程接收节点信息
4. 启动心跳线程定期广播/多播本地节点信息
5. 维护节点列表和状态
6. 触发相应回调（发现/丢失/更新）

心跳检测：
1. 定期发送本地节点信息
2. 更新其他节点的最后心跳时间
3. 超时未收到心跳的节点标记为离线

节点状态机：
UNKNOWN → ONLINE → SUSPECT → OFFLINE
         ↓________↑
```

### 2.3 数据结构/接口
```c
// 主要头文件：idcu/discovery/discovery.h

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

// 核心 API
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
```

### 2.4 跨平台适配
- **Windows/Linux**: 使用 idcu-network 库封装的跨平台 Socket API
- **多播支持**: 跨平台设置 IP_ADD_MEMBERSHIP/IP_DROP_MEMBERSHIP
- **多线程**: 使用 idcu-common 库封装的跨平台线程 API
- **时间戳**: 使用跨平台的毫秒级时间戳获取

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] UDP 广播发现正常工作
- [ ] UDP 多播发现正常工作
- [ ] 节点信息交换正常工作
- [ ] 心跳检测正常工作
- [ ] 节点列表管理正常工作
- [ ] 节点发现/丢失/更新回调可以正常触发
- [ ] 通过 ID/名称获取节点正常工作
- [ ] 标签和元数据管理正常工作
- [ ] 跨平台正常运行（Windows + Linux）

### 3.2 性能验收
- 节点发现延迟 ≤ 5 秒（局域网内）
- 支持 100+ 节点并发发现
- 心跳间隔可配置（默认 5 秒）
- 内存占用 ≤ 5MB（100 节点）
- CPU 占用 ≤ 5%（空闲状态）

### 3.3 异常验收
- [ ] 网络断开后节点正确标记为离线
- [ ] 节点超时后正确触发回调
- [ ] NULL 指针检查正确
- [ ] 无效参数返回明确错误码
- [ ] Socket 创建失败返回明确错误码

---

## 4. 执行计划

### 4.1 工期
6 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义和数据结构（1 小时）
- D1-60: 完成核心发现逻辑（1.5 小时）
- D1-150: 完成心跳检测和节点管理（1.5 小时）
- D1-240: 完成回调机制和工具函数（1 小时）
- D1-300: 完成测试和文档（0.5 小时）

### 4.3 人力
1 人（技能要求：C 语言 + 网络编程 + 跨平台）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，前缀 idcu_

### 5.2 测试要求
- 单元测试覆盖率 ≥ 70%
- 跨平台测试（Windows + Linux）

### 5.3 部署指引
- 编译命令：`cmake -B build && cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::discovery)`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：网络隔离导致无法发现节点  
应对：提供 UDP 广播和多播双模式，允许用户选择可用的模式

### 6.2 风险2
描述：节点频繁掉线影响稳定性  
应对：提供可配置的心跳间隔和超时时间，允许根据网络环境调整

---

## 7. 详细实现步骤

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