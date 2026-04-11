# idcu-distributed

IDCU Agent 的分布式节点支持库。

## 功能特性
- 节点发现和注册
- 节点间通信
- 领导者选举（类 Raft）
- 多级数据一致性
- 故障检测和恢复
- 消息广播和组播
- 线程安全实现
- CMake 构建配置

## 使用方法

```c
#include <idcu/distributed/distributed.h>

idcu_DistributedNode* ctx;
int ret = idcu_distributed_node_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // 启动节点
    ret = idcu_distributed_node_start(ctx);
    if (ret == IDCU_ERR_OK) {
        // 向节点发送消息
        const char* msg = "你好";
        idcu_distributed_send_to(ctx, 1, msg, strlen(msg));
        
        // 广播消息
        idcu_distributed_broadcast(ctx, msg, strlen(msg));
        
        // 获取领导者
        idcu_NodeId leader;
        idcu_distributed_get_leader(ctx, &leader);
        
        // 停止节点
        idcu_distributed_node_stop(ctx);
    }
    
    // 销毁
    idcu_distributed_node_destroy(ctx);
}
```

## API 参考
- `idcu_distributed_node_init()` - 初始化分布式节点
- `idcu_distributed_node_start()` - 启动分布式节点
- `idcu_distributed_node_stop()` - 停止分布式节点
- `idcu_distributed_node_destroy()` - 销毁分布式节点
- `idcu_distributed_send_to()` - 向特定节点发送消息
- `idcu_distributed_broadcast()` - 向所有节点广播消息
- `idcu_distributed_get_leader()` - 获取当前领导者节点 ID
- `idcu_distributed_get_node_info()` - 获取节点信息
- `idcu_distributed_list_nodes()` - 列出所有节点

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
