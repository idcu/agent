#ifndef IDCU_DISTRIBUTED_TYPES_H
#define IDCU_DISTRIBUTED_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_DISTRIBUTED_MAX_ITEMS 1024

typedef uint64_t idcu_NodeId;

typedef enum {
    IDCU_NODE_STATE_UNKNOWN = 0,
    IDCU_NODE_STATE_OFFLINE,
    IDCU_NODE_STATE_ONLINE,
    IDCU_NODE_STATE_LEADER,
    IDCU_NODE_STATE_FOLLOWER,
    IDCU_NODE_STATE_CANDIDATE
} idcu_NodeState;

typedef enum {
    IDCU_CONSISTENCY_STRONG = 0,
    IDCU_CONSISTENCY_EVENTUAL,
    IDCU_CONSISTENCY_QUORUM
} idcu_ConsistencyLevel;

typedef struct {
    idcu_NodeId id;
    char name[128];
    char address[256];
    uint16_t port;
    idcu_NodeState state;
    uint64_t last_heartbeat;
    uint64_t uptime;
    char metadata[1024];
} idcu_NodeInfo;

typedef struct idcu_Distributed_Context {
    idcu_NodeInfo nodes[IDCU_DISTRIBUTED_MAX_ITEMS];
    int node_count;
    idcu_NodeId self_id;
    int initialized;
    idcu_Mutex lock;
    uint64_t operation_count;
    uint64_t error_count;
} idcu_Distributed_Context;

#ifdef __cplusplus
}
#endif

#endif
