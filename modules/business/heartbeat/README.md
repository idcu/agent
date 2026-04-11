# idcu-heartbeat

Heartbeat module for IDCU Agent, providing:
- Periodic heartbeat sending
- Heartbeat receiving and validation
- Node status monitoring
- Heartbeat timeout detection
- Auto reconnect
- Heartbeat history
- Heartbeat statistics
- Multi-protocol support
- Custom heartbeat data

## Usage

```c
#include <idcu/heartbeat/heartbeat.h>

int main() {
    idcu_HeartbeatModule hb;
    int ret = idcu_heartbeat_module_init(&hb, "node-001", "Main Node");
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
    printf("Status: %d\n", status);
    
    idcu_heartbeat_module_stop(&hb);
    idcu_heartbeat_module_destroy(&hb);
    
    return 0;
}
```
