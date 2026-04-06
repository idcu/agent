# idcu-network

基础网络层库，提供 Socket 封装、TCP/UDP 服务器和客户端功能。

## 功能特性

- 跨平台 Socket 封装（Windows/Linux）
- TCP 和 UDP 协议支持
- 服务器和客户端功能
- 非阻塞 I/O 支持
- 连接管理

## 快速开始

```c
#include "idcu/network/network_layer.h"

int main() {
    idcu_network_init();
    
    idcu_NetworkServer server;
    idcu_network_server_create(&server, IDCU_NET_PROTO_TCP, "0.0.0.0", 8080);
    idcu_network_server_listen(&server);
    
    // 处理连接...
    
    idcu_network_server_destroy(&server);
    idcu_network_cleanup();
    return 0;
}
```

## 构建

```bash
mkdir build && cd build
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
