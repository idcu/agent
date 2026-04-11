# idcu-distributed

Distributed node support library for IDCU Agent.

## Features
- Node discovery and registration
- Inter-node communication
- Leader election (Raft-like)
- Data consistency with multiple levels
- Failure detection and recovery
- Message broadcast and multicast
- Thread-safe implementation
- CMake build configuration

## Usage

```c
#include <idcu/distributed/distributed.h>

idcu_Distributed_Context* ctx;
int ret = idcu_distributed_node_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Start node
    ret = idcu_distributed_node_start(ctx);
    if (ret == IDCU_ERR_OK) {
        // Send message to node
        const char* msg = "Hello";
        idcu_distributed_send_to(ctx, 1, msg, strlen(msg));
        
        // Broadcast message
        idcu_distributed_broadcast(ctx, msg, strlen(msg));
        
        // Get leader
        idcu_NodeId leader;
        idcu_distributed_get_leader(ctx, &leader);
        
        // Stop node
        idcu_distributed_node_stop(ctx);
    }
    
    // Destroy
    idcu_distributed_node_destroy(ctx);
}
```

## API Reference
- `idcu_distributed_node_init()` - Initialize distributed node
- `idcu_distributed_node_start()` - Start distributed node
- `idcu_distributed_node_stop()` - Stop distributed node
- `idcu_distributed_node_destroy()` - Destroy distributed node
- `idcu_distributed_send_to()` - Send message to specific node
- `idcu_distributed_broadcast()` - Broadcast message to all nodes
- `idcu_distributed_get_leader()` - Get current leader node ID
- `idcu_distributed_get_node_info()` - Get node information
- `idcu_distributed_list_nodes()` - List all nodes

## Building

```bash
cmake -B build && cmake --build build
```

## License
MIT
