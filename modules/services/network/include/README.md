# Network - 网络模块

本模块提供网络通信和分布式节点功能。

## 模块内容

### network_layer.h/c - 网络层
基础网络 Socket 封装：
- TCP 和 UDP 协议支持
- Socket 创建和销毁
- Server 监听和接受连接
- 跨平台 (Windows/Linux) 实现

### distributed_node.h/c - 分布式节点
分布式系统节点管理：
- 节点信息管理 (ID, 名称, 地址, 端口)
- 节点状态 (ONLINE, OFFLINE, BUSY, UNHEALTHY)
- 节点添加和移除
- 节点间消息传递
- 按负载选择节点
- 心跳管理
- 最大支持 32 个节点

### node_discovery.h/c - 节点发现
自动发现网络中的其他节点：
- 节点发现事件回调
- 节点丢失事件回调
- 与分布式节点集成

## 使用示例

### 使用网络层
```c
#include "network/network_layer.h"

// 创建服务器
idcu_NetworkServer server;
idcu_network_server_create(&server, IDCU_NET_PROTO_TCP, "0.0.0.0", 8080);

// 创建客户端连接
idcu_NetworkSocket sock;
idcu_network_socket_create(&sock, IDCU_NET_PROTO_TCP);

idcu_network_socket_destroy(&sock);
idcu_network_server_destroy(&server);
```

### 使用分布式节点
```c
#include "network/distributed_node.h"

idcu_DistributedNode node;
idcu_distributed_node_init(&node, 1, "node-1", "192.168.1.100", 8080);

// 添加其他节点
idcu_distributed_node_add_node(&node, 2, "node-2", "192.168.1.101", 8080);

// 发送消息
uint8_t data[] = {1, 2, 3, 4};
idcu_distributed_node_send_message(&node, 2, 0, data, sizeof(data));

// 接收消息
idcu_NodeMessage msg;
idcu_distributed_node_recv_message(&node, &msg);

// 更新心跳
idcu_distributed_node_update_heartbeat(&node, 2);

idcu_distributed_node_destroy(&node);
```

### 使用节点发现
```c
#include "network/node_discovery.h"

void on_node_discovered(void* user_data, idcu_NodeInfo* node) {
    printf("发现节点: %s\n", node->name);
}

void on_node_lost(void* user_data, idcu_NodeInfo* node) {
    printf("丢失节点: %s\n", node->name);
}

idcu_NodeDiscovery disc;
idcu_node_discovery_init(&disc, &dist_node);
idcu_node_discovery_set_discovered_handler(&disc, on_node_discovered, NULL);
idcu_node_discovery_set_lost_handler(&disc, on_node_lost, NULL);

idcu_node_discovery_destroy(&disc);
```
