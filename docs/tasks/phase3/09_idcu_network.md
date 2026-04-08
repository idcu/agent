# 任务 3.9: idcu-network - 网络层库

## 目标

创建跨平台网络层库，支持：
- TCP 客户端和服务器
- UDP 通信
- Socket 管理
- 地址解析
- 超时设置
- 非阻塞 I/O
- Windows/Linux/macOS 跨平台

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-network/include/idcu/network
mkdir -p libs/idcu-network/src/idcu/network
mkdir -p libs/idcu-network/tests
mkdir -p libs/idcu-network/examples
```

### 2. 创建网络头文件 (network.h)

创建 `libs/idcu-network/include/idcu/network/network.h`：

```c
#ifndef IDCU_NETWORK_NETWORK_H
#define IDCU_NETWORK_NETWORK_H

#include "idcu/common/error_code.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET idcu_Socket;
    #define IDCU_INVALID_SOCKET INVALID_SOCKET
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int idcu_Socket;
    #define IDCU_INVALID_SOCKET (-1)
#endif

typedef enum
{
    IDCU_NET_PROTO_TCP = 0,
    IDCU_NET_PROTO_UDP
} idcu_NetProtocol;

typedef enum
{
    IDCU_NET_ADDR_IPV4 = 0,
    IDCU_NET_ADDR_IPV6
} idcu_NetAddrType;

typedef struct
{
    idcu_NetAddrType type;
    union
    {
        struct in_addr  v4;
        struct in6_addr v6;
    } addr;
    uint16_t port;
} idcu_NetAddress;

typedef struct
{
    idcu_Socket socket;
    idcu_NetProtocol protocol;
    idcu_NetAddress local_addr;
    idcu_NetAddress remote_addr;
    int is_connected;
    int is_blocking;
    int timeout_ms;
} idcu_TcpSocket;

typedef struct
{
    idcu_Socket socket;
    idcu_NetAddress local_addr;
    int is_bound;
    int timeout_ms;
} idcu_UdpSocket;

typedef struct
{
    idcu_Socket listen_socket;
    idcu_NetAddress listen_addr;
    int is_listening;
    int backlog;
} idcu_TcpServer;

int  idcu_network_init(void);
void idcu_network_cleanup(void);

int idcu_net_address_init(idcu_NetAddress* addr, idcu_NetAddrType type, const char* ip, uint16_t port);
int idcu_net_address_from_string(idcu_NetAddress* addr, const char* str);
int idcu_net_address_to_string(const idcu_NetAddress* addr, char* buffer, size_t buffer_size);
int idcu_net_address_resolve(idcu_NetAddress* addr, const char* hostname, uint16_t port);

int  idcu_tcp_socket_init(idcu_TcpSocket* sock);
void idcu_tcp_socket_destroy(idcu_TcpSocket* sock);
int  idcu_tcp_socket_connect(idcu_TcpSocket* sock, const idcu_NetAddress* addr);
int  idcu_tcp_socket_connect_timeout(idcu_TcpSocket* sock, const idcu_NetAddress* addr, int timeout_ms);
void idcu_tcp_socket_disconnect(idcu_TcpSocket* sock);
int  idcu_tcp_socket_send(idcu_TcpSocket* sock, const void* data, size_t len, size_t* sent);
int  idcu_tcp_socket_send_all(idcu_TcpSocket* sock, const void* data, size_t len);
int  idcu_tcp_socket_recv(idcu_TcpSocket* sock, void* buffer, size_t len, size_t* received);
int  idcu_tcp_socket_recv_exact(idcu_TcpSocket* sock, void* buffer, size_t len);
int  idcu_tcp_socket_set_blocking(idcu_TcpSocket* sock, int blocking);
int  idcu_tcp_socket_set_timeout(idcu_TcpSocket* sock, int timeout_ms);
int  idcu_tcp_socket_get_error(idcu_TcpSocket* sock);

int  idcu_tcp_server_init(idcu_TcpServer* server);
void idcu_tcp_server_destroy(idcu_TcpServer* server);
int  idcu_tcp_server_listen(idcu_TcpServer* server, const idcu_NetAddress* addr, int backlog);
void idcu_tcp_server_stop(idcu_TcpServer* server);
int  idcu_tcp_server_accept(idcu_TcpServer* server, idcu_TcpSocket* client);
int  idcu_tcp_server_accept_timeout(idcu_TcpServer* server, idcu_TcpSocket* client, int timeout_ms);

int  idcu_udp_socket_init(idcu_UdpSocket* sock);
void idcu_udp_socket_destroy(idcu_UdpSocket* sock);
int  idcu_udp_socket_bind(idcu_UdpSocket* sock, const idcu_NetAddress* addr);
void idcu_udp_socket_close(idcu_UdpSocket* sock);
int  idcu_udp_socket_send(idcu_UdpSocket* sock, const idcu_NetAddress* dest, const void* data, size_t len);
int  idcu_udp_socket_recv(idcu_UdpSocket* sock, idcu_NetAddress* source, void* buffer, size_t len, size_t* received);
int  idcu_udp_socket_set_timeout(idcu_UdpSocket* sock, int timeout_ms);
int  idcu_udp_socket_set_broadcast(idcu_UdpSocket* sock, int enable);

typedef struct
{
    idcu_TcpSocket** sockets;
    size_t count;
    size_t capacity;
} idcu_SocketSet;

int  idcu_socket_set_init(idcu_SocketSet* set, size_t capacity);
void idcu_socket_set_destroy(idcu_SocketSet* set);
int  idcu_socket_set_add(idcu_SocketSet* set, idcu_TcpSocket* sock);
int  idcu_socket_set_remove(idcu_SocketSet* set, idcu_TcpSocket* sock);
int  idcu_socket_set_select(idcu_SocketSet* read_set, idcu_SocketSet* write_set, idcu_SocketSet* except_set, int timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-network/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-network VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-network STATIC
    src/idcu/network/network.c
)

target_include_directories(idcu-network PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-network PRIVATE
    idcu::common
)

if(WIN32)
    target_link_libraries(idcu-network PRIVATE ws2_32)
endif()

add_library(idcu::network ALIAS idcu-network)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-network/module.yaml`：

```yaml
name: idcu-network
version: 1.0.0
description: Cross-platform network library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common

build:
  type: cmake
  targets:
    - idcu-network

headers:
  - idcu/network/network.h

features:
  - tcp_client: TCP client socket
  - tcp_server: TCP server socket
  - udp: UDP socket support
  - cross_platform: Windows/Linux/macOS support
  - address: Address resolution and manipulation
  - non_blocking: Non-blocking I/O support
  - timeout: Timeout settings
  - select: Socket set and select operations

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `libs/idcu-network/README.md`：

```markdown
# idcu-network

IDCU Agent 的跨平台网络层库。

## 功能特性

- **TCP 客户端**: TCP 客户端 Socket
- **TCP 服务器**: TCP 服务器 Socket
- **UDP 通信**: UDP Socket 支持
- **跨平台**: 支持 Windows/Linux/macOS
- **地址解析**: 地址解析和操作
- **非阻塞 I/O**: 非阻塞 I/O 支持
- **超时设置**: 超时设置
- **Socket 选择**: Socket 集合和 select 操作

## 快速开始

### 初始化网络库

```c
#include "idcu/network/network.h"

idcu_network_init();
```

### TCP 客户端

```c
idcu_TcpSocket sock;
idcu_tcp_socket_init(&sock);

idcu_NetAddress addr;
idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV4, "127.0.0.1", 8080);

idcu_tcp_socket_connect(&sock, &addr);

const char* data = "Hello, Server!";
idcu_tcp_socket_send_all(&sock, data, strlen(data));

char buffer[1024];
size_t received;
idcu_tcp_socket_recv(&sock, buffer, sizeof(buffer), &received);

idcu_tcp_socket_disconnect(&sock);
idcu_tcp_socket_destroy(&sock);
```

### TCP 服务器

```c
idcu_TcpServer server;
idcu_tcp_server_init(&server);

idcu_NetAddress addr;
idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV4, "0.0.0.0", 8080);

idcu_tcp_server_listen(&server, &addr, 10);

idcu_TcpSocket client;
idcu_tcp_server_accept(&server, &client);

char buffer[1024];
size_t received;
idcu_tcp_socket_recv(&client, buffer, sizeof(buffer), &received);

idcu_tcp_socket_disconnect(&client);
idcu_tcp_socket_destroy(&client);

idcu_tcp_server_stop(&server);
idcu_tcp_server_destroy(&server);
```

### UDP 通信

```c
idcu_UdpSocket sock;
idcu_udp_socket_init(&sock);

idcu_NetAddress local_addr;
idcu_net_address_init(&local_addr, IDCU_NET_ADDR_IPV4, "0.0.0.0", 9090);
idcu_udp_socket_bind(&sock, &local_addr);

idcu_NetAddress dest_addr;
idcu_net_address_init(&dest_addr, IDCU_NET_ADDR_IPV4, "127.0.0.1", 9091);

const char* data = "Hello, UDP!";
idcu_udp_socket_send(&sock, &dest_addr, data, strlen(data));

char buffer[1024];
idcu_NetAddress source_addr;
size_t received;
idcu_udp_socket_recv(&sock, &source_addr, buffer, sizeof(buffer), &received);

idcu_udp_socket_close(&sock);
idcu_udp_socket_destroy(&sock);
```

### 清理

```c
idcu_network_cleanup();
```

## API 文档

详见 [include/idcu/network/network.h](include/idcu/network/network.h)
```

## 验证检查清单

- [ ] 网络头文件已创建
- [ ] 网络实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] TCP 客户端可以连接和发送数据
- [ ] TCP 服务器可以监听和接受连接
- [ ] UDP Socket 可以发送和接收数据

## Git 提交

```bash
git add libs/idcu-network/
git commit -m "feat: add idcu-network library

- Add TCP client and server socket
- Add UDP socket support
- Add cross-platform (Windows/Linux/macOS) support
- Add address resolution and manipulation
- Add non-blocking I/O support
- Add timeout settings
- Add socket set and select operations
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 连接失败 | 地址或端口错误 | 检查服务器地址和端口 |
| 发送超时 | 网络拥堵 | 增加超时时间或检查网络 |
| Windows 初始化失败 | 未调用 idcu_network_init | 确保首先初始化网络库 |
