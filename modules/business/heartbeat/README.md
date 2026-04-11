# idcu-heartbeat

IDCU Agent 的心跳模块，提供：
- 周期性心跳发送
- 心跳接收和验证
- 节点状态监控
- 心跳超时检测
- 自动重连
- 心跳历史
- 心跳统计
- 多协议支持
- 自定义心跳数据

## 使用方法

```c
#include <idcu/heartbeat/heartbeat.h>

int main() {
    idcu_HeartbeatModule hb;
    int ret = idcu_heartbeat_module_init(&hb, "node-001", "主节点");
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_heartbeat_module_start(&hb);
    if (ret != IDCU_ERR_OK) {
        idcu_heartbeat_module_destroy(&hb);
        return 1;
    }
    
    idcu_heartbeat_module_send(&hb);
    
    idcu_HeartbeatStatus status = idcu_heartbeat_module_get_status(&hb);
    printf("状态: %d\n", status);
    
    idcu_heartbeat_module_stop(&hb);
    idcu_heartbeat_module_destroy(&hb);
    
    return 0;
}
```

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
