# idcu-network

IDCU 项目的基础网络层库，提供跨平台的 Socket 封装、TCP/UDP 服务器和客户端功能。

## 功能特性

- 跨平台 Socket 封装（Windows/Linux/macOS）
- TCP 和 UDP 协议支持
- 服务器和客户端功能
- 可配置超时和重试机制
- 自动重连支持
- 发送/接收重试
- 连接管理
- 线程安全的网络配置

## 快速开始

### 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 安装

```bash
cmake --install .
```

### 在其他项目中使用

```cmake
find_package(idcu-common REQUIRED)
find_package(idcu-log REQUIRED)
find_package(idcu-network REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::network)
```

### 简单的 TCP 服务器示例

```c
#include "idcu/network/network_layer.h"
#include <stdio.h>

int main() {
    idcu_network_init();

    idcu_NetworkServer server;
    idcu_network_server_create(&server, IDCU_NET_PROTO_TCP, "0.0.0.0", 8080);
    idcu_network_server_listen(&server);

    printf("Server listening on 0.0.0.0:8080\n");

    idcu_NetworkSocket client;
    while (1) {
        idcu_network_server_accept(&server, &client);
        printf("Client connected: %s:%d\n", client.remote_addr, client.remote_port);
        idcu_network_socket_close(&client);
    }

    idcu_network_server_destroy(&server);
    idcu_network_cleanup();
    return 0;
}
```

### 简单的 TCP 客户端示例

```c
#include "idcu/network/network_layer.h"
#include <stdio.h>
#include <string.h>

int main() {
    idcu_network_init();

    idcu_NetworkSocket sock;
    idcu_network_socket_create(&sock, IDCU_NET_PROTO_TCP);
    idcu_network_socket_connect(&sock, "127.0.0.1", 8080);

    const char* message = "Hello from TCP client!";
    size_t sent;
    idcu_network_socket_send(&sock, message, strlen(message), &sent);

    idcu_network_socket_destroy(&sock);
    idcu_network_cleanup();
    return 0;
}
```

## API 文档

### 初始化和清理

```c
#include "idcu/network/network_layer.h"

// 初始化网络层
int idcu_network_init(void);

// 清理网络层资源
void idcu_network_cleanup(void);
```

### 常量定义

```c
// 协议类型
#define IDCU_NET_PROTO_TCP 0  // TCP 协议
#define IDCU_NET_PROTO_UDP 1  // UDP 协议

// 地址最大长度
#define IDCU_ADDR_MAX 64

// 默认配置
#define IDCU_NET_DEFAULT_TIMEOUT_MS 30000       // 默认超时 30 秒
#define IDCU_NET_DEFAULT_MAX_RETRIES 3          // 默认最大重试次数
#define IDCU_NET_DEFAULT_RETRY_DELAY_MS 1000    // 默认重试延迟 1 秒
#define IDCU_NET_DEFAULT_RECONNECT_DELAY_MS 2000 // 默认重连延迟 2 秒
```

### 配置结构

```c
typedef struct {
    uint32_t connect_timeout_ms;     // 连接超时（毫秒）
    uint32_t send_timeout_ms;        // 发送超时（毫秒）
    uint32_t recv_timeout_ms;        // 接收超时（毫秒）
    uint32_t max_retries;            // 最大重试次数
    uint32_t retry_delay_ms;         // 重试延迟（毫秒）
    uint32_t reconnect_delay_ms;     // 重连延迟（毫秒）
    uint32_t retry_count;            // 当前重试计数
} idcu_NetworkConfig;
```

### Socket 操作

```c
// 创建 Socket
int idcu_network_socket_create(idcu_NetworkSocket* sock, int protocol);

// 销毁 Socket
void idcu_network_socket_destroy(idcu_NetworkSocket* sock);

// 连接到服务器
int idcu_network_socket_connect(idcu_NetworkSocket* sock, const char* address, uint16_t port);

// 绑定到地址
int idcu_network_socket_bind(idcu_NetworkSocket* sock, const char* address, uint16_t port);

// 发送数据（TCP）
int idcu_network_socket_send(idcu_NetworkSocket* sock, const void* data, size_t len, size_t* sent);

// 接收数据（TCP）
int idcu_network_socket_recv(idcu_NetworkSocket* sock, void* data, size_t len, size_t* received);

// 发送数据到指定地址（UDP）
int idcu_network_socket_sendto(idcu_NetworkSocket* sock, const void* data, size_t len, 
                               const char* address, uint16_t port, size_t* sent);

// 从任意地址接收数据（UDP）
int idcu_network_socket_recvfrom(idcu_NetworkSocket* sock, void* data, size_t len,
                                 char* address, size_t addr_len, uint16_t* port, size_t* received);

// 关闭 Socket
int idcu_network_socket_close(idcu_NetworkSocket* sock);
```

### 服务器操作

```c
// 创建服务器
int idcu_network_server_create(idcu_NetworkServer* server, int protocol, const char* address, uint16_t port);

// 销毁服务器
void idcu_network_server_destroy(idcu_NetworkServer* server);

// 开始监听
int idcu_network_server_listen(idcu_NetworkServer* server);

// 接受连接
int idcu_network_server_accept(idcu_NetworkServer* server, idcu_NetworkSocket* client_sock);

// 关闭服务器
int idcu_network_server_close(idcu_NetworkServer* server);
```

### 高级配置和操作

```c
// 设置 Socket 配置
int idcu_network_socket_set_config(idcu_NetworkSocket* sock, const idcu_NetworkConfig* config);

// 获取 Socket 配置
int idcu_network_socket_get_config(idcu_NetworkSocket* sock, idcu_NetworkConfig* config);

// 设置超时
int idcu_network_socket_set_timeout(idcu_NetworkSocket* sock, uint32_t timeout_ms);

// 重新连接
int idcu_network_socket_reconnect(idcu_NetworkSocket* sock);

// 带重试的发送
int idcu_network_socket_send_with_retry(idcu_NetworkSocket* sock, const void* data, size_t len, size_t* sent);

// 带重试的接收
int idcu_network_socket_recv_with_retry(idcu_NetworkSocket* sock, void* data, size_t len, size_t* received);
```

## 示例

库提供了以下示例代码：

- [TCP 服务器示例](examples/example_tcp_server.c) - 演示 TCP 服务器的基本使用
- [TCP 客户端示例](examples/example_tcp_client.c) - 演示 TCP 客户端的基本使用
- [UDP 示例](examples/example_udp.c) - 演示 UDP Socket 的使用

### 编译和运行示例

```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .

# 运行 TCP 服务器（在一个终端）
./example_tcp_server

# 运行 TCP 客户端（在另一个终端）
./example_tcp_client
```

## 使用指南

### TCP 服务器完整流程

1. 初始化网络层
2. 创建服务器对象
3. 绑定并监听
4. 接受连接
5. 处理客户端
6. 清理资源

```c
#include "idcu/network/network_layer.h"
#include <stdio.h>

int main() {
    // 1. 初始化
    idcu_network_init();

    // 2. 创建服务器
    idcu_NetworkServer server;
    int ret = idcu_network_server_create(&server, IDCU_NET_PROTO_TCP, "0.0.0.0", 8080);
    if (ret != IDCU_ERR_OK) {
        printf("Server create failed\n");
        return 1;
    }

    // 3. 监听
    ret = idcu_network_server_listen(&server);
    if (ret != IDCU_ERR_OK) {
        printf("Listen failed\n");
        idcu_network_server_destroy(&server);
        idcu_network_cleanup();
        return 1;
    }

    printf("Server ready\n");

    // 4. 接受连接
    idcu_NetworkSocket client;
    while (1) {
        ret = idcu_network_server_accept(&server, &client);
        if (ret != IDCU_ERR_OK) break;
        
        // 5. 处理客户端
        char buffer[1024];
        size_t received;
        idcu_network_socket_recv(&client, buffer, sizeof(buffer), &received);
        printf("Received: %.*s\n", (int)received, buffer);
        
        idcu_network_socket_close(&client);
    }

    // 6. 清理
    idcu_network_server_destroy(&server);
    idcu_network_cleanup();
    return 0;
}
```

### TCP 客户端完整流程

1. 初始化网络层
2. 创建 Socket
3. 连接服务器
4. 发送/接收数据
5. 清理资源

```c
#include "idcu/network/network_layer.h"
#include <stdio.h>
#include <string.h>

int main() {
    // 1. 初始化
    idcu_network_init();

    // 2. 创建 Socket
    idcu_NetworkSocket sock;
    int ret = idcu_network_socket_create(&sock, IDCU_NET_PROTO_TCP);
    if (ret != IDCU_ERR_OK) {
        printf("Socket create failed\n");
        return 1;
    }

    // 3. 连接
    ret = idcu_network_socket_connect(&sock, "127.0.0.1", 8080);
    if (ret != IDCU_ERR_OK) {
        printf("Connect failed\n");
        idcu_network_socket_destroy(&sock);
        idcu_network_cleanup();
        return 1;
    }

    // 4. 发送数据
    const char* msg = "Hello, Server!";
    size_t sent;
    idcu_network_socket_send(&sock, msg, strlen(msg), &sent);

    // 5. 清理
    idcu_network_socket_destroy(&sock);
    idcu_network_cleanup();
    return 0;
}
```

### 自定义配置

```c
idcu_NetworkConfig config = {
    .connect_timeout_ms = 5000,      // 5 秒连接超时
    .send_timeout_ms = 10000,        // 10 秒发送超时
    .recv_timeout_ms = 10000,        // 10 秒接收超时
    .max_retries = 5,                 // 最多重试 5 次
    .retry_delay_ms = 500,            // 重试间隔 500 毫秒
    .reconnect_delay_ms = 3000        // 重连延迟 3 秒
};

idcu_network_socket_set_config(&sock, &config);
```

## 测试

```bash
cd build
ctest
```

## 依赖

- idcu-common - IDCU 基础通用组件库
- idcu-log - IDCU 日志库

## 许可证

详见项目根目录的 LICENSE 文件。
