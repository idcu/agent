# idcu-distributed

分布式节点库，提供节点管理和消息通信功能。

## 功能特性

- 分布式节点管理
- 节点间消息通信
- 负载均衡选择
- 节点状态管理
- 跨平台支持（Windows/Linux）

## 快速开始

```c
#include "idcu/distributed/distributed.h"

int main() {
    idcu_DistributedNode node;
    idcu_distributed_node_init(&amp;node, 12345, "node1", "127.0.0.1", 8080);
    
    idcu_distributed_node_start_server(&amp;node);
    
    idcu_distributed_node_add_node(&amp;node, 67890, "node2", "127.0.0.1", 8081);
    idcu_distributed_node_connect_to_node(&amp;node, 67890);
    
    const char* msg = "Hello from node1";
    idcu_distributed_node_send_message(&amp;node, 67890, 1, msg, strlen(msg));
    
    idcu_distributed_node_process(&amp;node);
    
    idcu_distributed_node_stop_server(&amp;node);
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
