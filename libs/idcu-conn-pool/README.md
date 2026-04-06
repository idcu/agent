# idcu-conn-pool

连接池库，提供网络连接复用和管理功能。

## 功能特性

- TCP/UDP 连接池管理
- 连接复用，减少连接建立开销
- 空闲连接自动清理
- 线程安全
- 跨平台支持（Windows/Linux）

## 快速开始

```c
#include "idcu/conn_pool/conn_pool.h"

int main() {
    idcu_ConnectionPool pool;
    idcu_connection_pool_init(&amp;pool, "my_pool", IDCU_NET_PROTO_TCP);
    
    idcu_ConnectionHandle handle;
    int ret = idcu_connection_pool_acquire(&amp;pool, "127.0.0.1", 8080, &amp;handle);
    if (ret == IDCU_ERR_SUCCESS) {
        // 使用连接发送/接收数据
        const char* msg = "Hello";
        size_t sent;
        idcu_connection_pool_send(&amp;handle, msg, strlen(msg), &amp;sent);
        
        // 释放连接回池
        idcu_connection_pool_release(&amp;handle);
    }
    
    idcu_connection_pool_destroy(&amp;pool);
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
