# idcu-discovery

节点发现库，提供网络节点自动发现功能。

## 功能特性

- UDP 广播节点发现
- 节点心跳检测
- 节点丢失检测
- 回调机制通知节点发现/丢失
- 跨平台支持（Windows/Linux）

## 快速开始

```c
#include "idcu/discovery/discovery.h"

void on_node_discovered(void* user_data, idcu_NodeInfo* node) {
    printf("Discovered node: %s (ID: %llu)\n", node-&gt;name, (unsigned long long)node-&gt;node_id);
}

void on_node_lost(void* user_data, idcu_NodeInfo* node) {
    printf("Lost node: %s (ID: %llu)\n", node-&gt;name, (unsigned long long)node-&gt;node_id);
}

int main() {
    idcu_DistributedNode node;
    idcu_distributed_node_init(&amp;node, 12345, "node1", "127.0.0.1", 8080);
    
    idcu_NodeDiscovery disc;
    idcu_node_discovery_init(&amp;disc, &amp;node);
    idcu_node_discovery_set_discovered_handler(&amp;disc, on_node_discovered, NULL);
    idcu_node_discovery_set_lost_handler(&amp;disc, on_node_lost, NULL);
    idcu_node_discovery_start(&amp;disc);
    
    while (1) {
        idcu_node_discovery_poll(&amp;disc);
        // 等待一段时间
    }
    
    idcu_node_discovery_stop(&amp;disc);
    idcu_node_discovery_destroy(&amp;disc);
    idcu_distributed_node_destroy(&amp;node);
    return 0;
}
```

## 构建

```bash
mkdir build &amp;&amp; cd build
cmake ..
cmake --build .
```

## 安装

```bash
cmake --install .
```

## 依赖

- idcu-common
- idcu-log
- idcu-network
- idcu-distributed
