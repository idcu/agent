# 分布式节点通信示例

## 功能说明

本示例演示如何使用IDCU的分布式节点功能，实现多个节点之间的自动发现、消息通信和数据同步。

### 主要功能
- 节点初始化和唯一标识
- 对等节点管理（添加/移除）
- 点对点消息发送
- 广播消息
- 节点自动发现
- 心跳检测和健康检查
- 跨平台支持

## 编译方式

### 编译示例程序

```bash
# 使用CMake编译
mkdir -p build && cd build
cmake ..
make distributed_node_example

# 或直接使用gcc
gcc -o distributed_node_example distributed_node_example.c \
    -I./libs/idcu-distributed/include \
    -I./libs/idcu-discovery/include \
    -I./libs/idcu-log/include \
    -I./libs/idcu-common/include \
    -L./build/libs \
    -lidcu-distributed -lidcu-discovery -lidcu-log -lidcu-common
```

## 运行步骤

### 1. 启动第一个节点（协调者）

```bash
# Windows
distributed_node_example.exe coordinator 9000

# Linux
./distributed_node_example coordinator 9000
```

### 2. 启动第二个节点（工作节点1）

在另一个终端运行：

```bash
# Windows
distributed_node_example.exe worker-1 9001

# Linux
./distributed_node_example worker-1 9001
```

### 3. 启动第三个节点（工作节点2）

在第三个终端运行：

```bash
# Windows
distributed_node_example.exe worker-2 9002

# Linux
./distributed_node_example worker-2 9002
```

节点会自动发现彼此并建立连接。

## 核心代码解读

### 1. 分布式节点初始化

```c
idcu_DistributedNode node;
int ret = idcu_distributed_node_init(&node, port, node_name, "127.0.0.1", port);
```

- 初始化分布式节点
- 分配唯一节点ID
- 设置节点名称和网络地址
- 初始化节点列表和通信通道

### 2. 添加对等节点

```c
int ret = idcu_distributed_node_add_node(&node, 1001, "worker-node-1", "192.168.1.101", 8001);
```

- 手动添加已知的对等节点
- 指定节点ID、名称、IP和端口
- 建立连接并开始心跳检测

### 3. 点对点消息发送

```c
idcu_DistributedMessage msg;
memset(&msg, 0, sizeof(msg));
msg.source_node_id = node->self_node_id;
msg.target_node_id = target_node_id;
msg.message_type = IDCU_DISTRIBUTED_MSG_TYPE_DATA;
msg.data_length = (uint32_t)strlen(message);
memcpy(msg.data, message, msg.data_length);

int ret = idcu_distributed_send_message(node, &msg);
```

- 构建消息结构
- 指定源节点和目标节点
- 设置消息类型和数据
- 发送到指定节点

### 4. 广播消息

```c
idcu_DistributedMessage msg;
msg.target_node_id = 0;  // 0表示广播
msg.message_type = IDCU_DISTRIBUTED_MSG_TYPE_BROADCAST;

int ret = idcu_distributed_broadcast_message(node, &msg);
```

- 目标节点ID设为0表示广播
- 消息会发送给所有已知的对等节点
- 适合发送系统通知和全局事件

### 5. 节点发现服务

```c
idcu_DiscoveryService discovery;
int ret = idcu_discovery_init(&discovery, port + 100);

// 发现服务会自动：
// - 广播节点存在公告
// - 监听其他节点的公告
// - 自动添加新发现的节点
// - 检测离线节点并移除

idcu_discovery_destroy(&discovery);
```

- 初始化节点发现服务
- 使用组播（Multicast）进行节点发现
- 自动维护节点列表
- 支持心跳超时检测

### 6. 节点信息结构

```c
typedef struct {
    uint64_t node_id;        // 唯一节点ID
    char node_name[64];      // 节点名称
    char ip_address[64];     // IP地址
    uint16_t port;           // 端口号
    bool is_connected;       // 连接状态
    // ... 更多字段
} NodeInfo;
```

存储节点的完整信息，包括网络地址和状态。

## 消息类型

| 类型 | 描述 | 用途 |
|------|------|------|
| `DATA` | 数据消息 | 传输业务数据 |
| `BROADCAST` | 广播消息 | 发送给所有节点 |
| `HEARTBEAT` | 心跳消息 | 检测节点存活状态 |
| `COMMAND` | 命令消息 | 发送控制指令 |
| `RESPONSE` | 响应消息 | 回复请求 |
| `SYNC` | 同步消息 | 数据同步 |

## 节点发现机制

### 组播发现（Multicast）

- 默认组播地址：`239.255.0.1`
- 默认组播端口：取决于配置
- 公告间隔：5秒
- 节点超时：15秒

### 发现流程

1. 节点启动后定期发送组播公告
2. 其他节点收到公告后检查是否已认识该节点
3. 如新节点则自动添加到对等列表
4. 开始心跳检测
5. 如果超过超时时间未收到公告则移除节点

## 部署架构建议

### 主从架构

```
                    ┌─────────────┐
                    │  Master     │
                    │  (Coordinator)│
                    └──────┬──────┘
                           │
            ┌──────────────┼──────────────┐
            │              │              │
    ┌───────▼──────┐ ┌────▼─────┐ ┌─────▼──────┐
    │   Worker 1   │ │ Worker 2 │ │  Worker 3  │
    └──────────────┘ └──────────┘ └────────────┘
```

- Master节点：协调、任务分发、状态汇总
- Worker节点：执行具体任务、上报状态

### 对等网络架构

```
    ┌──────────┐          ┌──────────┐
    │  Node A  │◄────────►│  Node B  │
    └────┬─────┘          └────┬─────┘
         │                       │
         │                       │
    ┌────▼─────┐          ┌────▼─────┐
    │  Node C  │◄────────►│  Node D  │
    └──────────┘          └──────────┘
```

- 所有节点地位平等
- 任意节点间可直接通信
- 无单点故障

## 扩展建议

1. **数据一致性**: 添加Raft或Paxos共识算法
2. **数据分片**: 支持分布式数据存储和分片
3. **负载均衡**: 自动在节点间分配任务
4. **故障转移**: 节点故障时自动转移任务
5. **安全通信**: 添加TLS加密和节点认证
6. **消息队列**: 集成持久化消息队列
7. **监控面板**: 添加Web界面监控集群状态

## 依赖关系

- `idcu-distributed`: 分布式节点库
- `idcu-discovery`: 节点发现库
- `idcu-log`: 日志系统
- `idcu-common`: 通用工具和错误码
- 网络库（Socket）
